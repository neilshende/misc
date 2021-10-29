#include <fcntl.h>
#include <grpcpp/grpcpp.h>
#include <libgen.h>
#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#include "config_cli_reader.h"
#include "global_config.h"
#include "libmvm/src/libmvm_impl.h"
#include "libmvm/src/libmvm_proto.h"
#include "mvm_error.h"
#include "mvm_error_proto.h"
#include "mvmlog.h"
#include "protos/google/rpc/status.pb.h"
#include "protos/libmvm.grpc.pb.h"
#include "protos/libmvm.pb.h"
#include "subproc_invoke.h"
class LibmvmGrpcServiceImpl : public memverge::libmvm::v1::LibmvmService::Service {
 public:
  LibmvmGrpcServiceImpl(std::string libmvm_bin_location, int log_fd)
      : libmvm_bin_location_(std::move(libmvm_bin_location)), log_fd_(log_fd) {}

  ~LibmvmGrpcServiceImpl() {
    if (log_fd_ != -1) {
      close(log_fd_);
    }
  }

  grpc::Status GetMvmDevices(
      grpc::ServerContext *context, const google::protobuf::Empty *request,
      memverge::libmvm::v1::GetMvmDevicesResponse *response) override {
    return LibmvmMethodSubprocCall("GetMvmDevices", context, request, response);
  }

 private:
  grpc::Status LibmvmMethodSubprocCall(const std::string &method,
                                       grpc::ServerContext *context,
                                       const google::protobuf::Message *req,
                                       google::protobuf::Message *res) {
    mvmapi_context mvm_ctx;
    if (!ExtractContext(context->client_metadata(), &mvm_ctx)) {
      return grpc::Status(grpc::INVALID_ARGUMENT, "invalid context");
    }
    std::string in_params;
    if (!req->SerializeToString(&in_params)) {
      return grpc::Status(grpc::INVALID_ARGUMENT, "invalid request");
    }
    std::string uid = std::to_string(mvm_ctx.uid);
    std::string use_multicast = mvm_ctx.use_multicast ? "1" : "0";
    std::string out;
    int r = SubprocInvoke(
        libmvm_bin_location_ + "/libmvm_bin", &in_params, &out,
        {std::to_string(log_fd_).c_str(), log_trace ? "1" : "0", method.c_str(),
         uid.c_str(), mvm_ctx.socket_path, mvm_ctx.data_dir, mvm_ctx.lib_dir,
         mvm_ctx.mvsnap_log, mvm_ctx.mvmcli_path, mvm_ctx.tmp_exec_dir,
         mvm_ctx.mvsnap_path, use_multicast.c_str(), mvm_ctx.mcast_addr},
        {log_fd_});
    if (r == LIBMVM_NO_ERR) {
      bool r2 = res->ParseFromString(out);
      assert(r2);
      return {};
    } else if (r == -1) {
      MvmErrorPtr error = MvmError::CreateLibmvmError(LIBMVM_ERR_SYSTEM,
                                                      "failed libmvm subprocess invoke");
      memverge::MvmErrorProto error_proto = PackMvmErrorProto(error);
      google::rpc::Status details;
      details.set_code(grpc::INTERNAL);
      details.set_message("failed libmvm subprocess invoke");
      details.add_details()->PackFrom(error_proto);
      std::string details_str;
      bool r2 = details.SerializeToString(&details_str);
      assert(r2);
      return grpc::Status(grpc::INTERNAL, "failed libmvm subprocess invoke", details_str);
    } else {
      google::rpc::Status details;
      bool r2 = details.ParseFromString(out);
      if (!r2) {
        MvmErrorPtr error =
            MvmError::CreateLibmvmError(LibmvmError(r), "Error without output parameter");
        memverge::MvmErrorProto error_proto = PackMvmErrorProto(error);
        details.set_code(grpc::INTERNAL);
        details.set_message("Error without output parameter");
        details.add_details()->PackFrom(error_proto);
        std::string details_str;
        r2 = details.SerializeToString(&details_str);
        assert(r2);
        return grpc::Status(grpc::INTERNAL, details.message(), details_str);
      } else {
        return grpc::Status(grpc::INTERNAL, details.message(), out);
      }
    }
  }

  static bool ExtractContext(const std::multimap<grpc::string_ref, grpc::string_ref> &md,
                             mvmapi_context *mvm_ctx) {
    auto it = md.find(kMdKeyUid);
    if (it != md.end()) {
      try {
        mvm_ctx->uid = std::stoul(std::string(it->second.begin(), it->second.end()));
      } catch (const std::exception &e) {
        return false;
      }
    }
    it = md.find(kMdKeyUseMulticast);
    if (it != md.end()) {
      mvm_ctx->use_multicast = (std::string(it->second.begin(), it->second.end()) == "1");
    }
    if (!ExtractStringContext(md, kMdKeySocketPath, mvm_ctx->socket_path,
                              sizeof(mvm_ctx->socket_path)))
      return false;
    if (!ExtractStringContext(md, kMdKeyDataDir, mvm_ctx->data_dir,
                              sizeof(mvm_ctx->data_dir)))
      return false;
    if (!ExtractStringContext(md, kMdKeyLibDir, mvm_ctx->lib_dir,
                              sizeof(mvm_ctx->lib_dir)))
      return false;
    if (!ExtractStringContext(md, kMdKeyMvsnapLog, mvm_ctx->mvsnap_log,
                              sizeof(mvm_ctx->mvsnap_log)))
      return false;
    if (!ExtractStringContext(md, kMdKeyMvmcliPath, mvm_ctx->mvmcli_path,
                              sizeof(mvm_ctx->mvmcli_path)))
      return false;
    if (!ExtractStringContext(md, kMdKeyTmpExecDir, mvm_ctx->tmp_exec_dir,
                              sizeof(mvm_ctx->tmp_exec_dir)))
      return false;
    if (!ExtractStringContext(md, kMdKeyMvsnapPath, mvm_ctx->mvsnap_path,
                              sizeof(mvm_ctx->mvsnap_path)))
      return false;
    if (!ExtractStringContext(md, kMdKeyMcastAddr, mvm_ctx->mcast_addr,
                              sizeof(mvm_ctx->mcast_addr)))
      return false;
    return true;
  }

  static bool ExtractStringContext(
      const std::multimap<grpc::string_ref, grpc::string_ref> &md,
      const std::string &field, char *buf, size_t size) {
    auto it = md.find(field);
    if (it == md.end()) return true;
    if (it->second.length() >= size) return false;
    memcpy(buf, it->second.data(), it->second.length());
    buf[it->second.length()] = '\0';
    return true;
  }

  static grpc::Status CreateGrpcStatus(int libmvm_error, LibmvmInterface *libmvm) {
    MvmErrorPtr error =
        MvmError::CreateLibmvmError(LibmvmError(libmvm_error), libmvm->GetLastErrorMsg());
    memverge::MvmErrorProto error_proto = PackMvmErrorProto(error);
    std::string details;
    if (!error_proto.SerializeToString(&details)) {
      details.clear();
    }
    return grpc::Status(grpc::INTERNAL, libmvm->GetLastErrorMsg(), details);
  }

  std::string libmvm_bin_location_;
  int log_fd_ = -1;
};

static void Daemonize() {
  pid_t pid = fork();
  if (pid < 0) {
    exit(1);
  } else if (pid > 0) {
    exit(0);
  }
  if (setsid() < 0) {
    exit(1);
  }
  signal(SIGHUP, SIG_IGN);
  pid = fork();
  if (pid < 0) {
    exit(1);
  } else if (pid > 0) {
    exit(0);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_RDWR);
}

static bool GetGidByName(const std::string &group_name, gid_t *gid) {
  struct group grp;
  struct group *result;
  int buf_size = sysconf(_SC_GETGR_R_SIZE_MAX);
  if (buf_size < 0) buf_size = 1024;
  char *buf = nullptr;
  int r = -1;
  while (true) {
    buf = static_cast<char *>(realloc(buf, buf_size));
    if (buf == nullptr) {
      return false;
    }
    r = getgrnam_r(group_name.c_str(), &grp, buf, buf_size, &result);
    if (r == 0 || errno != ERANGE) break;
    buf_size *= 2;
  }
  free(buf);
  if (r != 0) return false;

  *gid = grp.gr_gid;
  return true;
}

int main(int argc, char *argv[]) {
  /* the service is required to run as root */
  if (geteuid() != 0) {
    fprintf(stderr, "Please run with root privilege.\n");
    return 1;
  }

  /* parse config parameters */
  std::string error;
  std::string config_file_path = kConfigFilePath;
  ConfigReader config_reader(kConfigParams, "/etc/memverge/mvsvcd.yml", "config", argc,
                             argv, &error);
  if (!error.empty()) {
    fprintf(stderr, "%s\n", error.c_str());
    fprintf(stderr, "Usage: mvsvcd OPTIONS\n");
    fprintf(stderr, "OPTIONS list:\n");
    fprintf(stderr, "%s\n", config_reader.PrintUsage().c_str());
    return 1;
  }

  /* daemonize */
  if (config_reader.GetBoolParam("daemonize")) {
    Daemonize();
  }

  /* setup log */
  log_trace = config_reader.GetBoolParam("log-trace");
  std::string log_dir = GlobalConfig::Instance()->GetLogDir();
  int log_fd = -1;
  if (!log_dir.empty()) {
    log_dir += '/';
    log_dir += kLogFileName;
    OpenLogFile(log_dir.c_str(), false);
    /* Get a dup log file fd without O_CLOEXEC for libmvm_bin */
    if (mvmlog_fd > 0) {
      log_fd = dup(mvmlog_fd);
    }
    /* change the group owner of the log file to "mvmm" */
    gid_t mvmm_gid;
    if (!GetGidByName("mvmm", &mvmm_gid)) {
      fprintf(stderr, "Cannot find group mvmm\n");
      return 1;
    }
    chown(log_dir.c_str(), -1, mvmm_gid);
  }

  /* always use the libmvm_bin in the same directory as the current process */
  char buf[PATH_MAX];
  int len = readlink("/proc/self/exe", buf, PATH_MAX);
  if (len < 0) {
    fprintf(stderr, "Failed to read the path of mvsvcd: %m\n");
    return 1;
  } else if (len >= PATH_MAX) {
    buf[PATH_MAX - 1] = '\0';
    buf[PATH_MAX - 2] = buf[PATH_MAX - 3] = buf[PATH_MAX - 4] = '.';
    fprintf(stderr, "mvsvcd path %s too long (%d vs %d)\n", buf, len, PATH_MAX - 1);
    return 1;
  }
  buf[len] = '\0';
  LibmvmGrpcServiceImpl libmvm_grpc_service(dirname(buf), log_fd);

  /* start the server */
  grpc::ServerBuilder builder;
  builder.RegisterService(&libmvm_grpc_service);
  builder.AddListeningPort(
      std::string("unix:") + config_reader.GetStringParam("socket-path"),
      grpc::InsecureServerCredentials());
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (server == nullptr) {
    fprintf(stderr, "Failed to create server.\n");
    return 1;
  }
  if (chmod(config_reader.GetStringParam("socket-path").c_str(), 0777) != 0) {
    fprintf(stderr, "Failed to set permission of socket.\n");
    return 1;
  }

  /* setup signal handers for terminating the server */
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SignalHandler;
  sigaction(SIGINT, &action, nullptr);
  sigaction(SIGTERM, &action, nullptr);

  /* wait until the server is signaled to be terminated */
  while (!stop_server) {
    pause();
  }
  server->Shutdown();
  server->Wait();
  return 0;
}

