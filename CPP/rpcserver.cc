#include <functional>
#include <string>

#include <setjmp.h>
#include <signal.h>
#include <execinfo.h>

#include "global_config.h"
#include "libmvm/src/libmvm_impl.h"
#include "libmvm/src/libmvm_proto.h"
#include "mvm_error.h"
#include "mvm_error_proto.h"
#include "mvmlog.h"
#include "protos/google/rpc/status.pb.h"
#include "protos/libmvm.pb.h"
#include "scope_guard.h"

#include <grpcpp/grpcpp.h>
static auto empty_res_fn = []() { return google::protobuf::Empty(); };
static auto empty_err_out_fn = []() { return nullptr; };

/**
 * Make a libmvm request. The libmvm call is wrapped in \p req_fn which returns the return
 * value of req_fn. If the call is successful, \p res_fn is used to pack the output
 * parameters into string \p out; otherwise, an error object with details of the error
 * (libmvm error code, error message) is packed into string \p out.
 */
template <typename ReqType, typename ResType>
static int LibmvmCall(LibmvmInterface *libmvm, std::function<int(const ReqType &)> req_fn,
                      std::function<ResType()> res_fn, 
                      std::function<std::unique_ptr<google::protobuf::Message>()> err_out_fn, 
                      const std::string &in_params, std::string *out) {
  ReqType req;
  if (!req.ParseFromString(in_params)) {
    MVMLOG("Failed parsing input parameters\n");
    return -1;
  }
  int r = req_fn(req);
  if (r != 0) {
    MvmErrorPtr error = MvmError::CreateLibmvmError(static_cast<LibmvmError>(r),
                                                    libmvm->GetLastErrorMsg());
    memverge::MvmErrorProto error_proto = PackMvmErrorProto(error);
    google::rpc::Status details;
    details.set_code(grpc::INTERNAL);
    details.set_message(libmvm->GetLastErrorMsg().empty() ? libmvm_errxlat(r)
                                                          : libmvm->GetLastErrorMsg());
    details.add_details()->PackFrom(error_proto);
    auto err_out_ptr = err_out_fn();
    if (err_out_ptr != nullptr) {
      details.add_details()->PackFrom(*err_out_ptr);
    }
    bool r2 = details.SerializeToString(out);
    assert(r2);
  } else {
    ResType res = res_fn();
    bool r2 = res.SerializeToString(out);
    assert(r2);
  }
  return r;
}

static int GetMvmDevices(LibmvmInterface *libmvm, const mvmapi_context *context,
                         const std::string &in_params, std::string *out) {
  mvm_device *devices;
  size_t number;
  return LibmvmCall<google::protobuf::Empty,
                    memverge::libmvm::v1::GetMvmDevicesResponse>(
      libmvm,
      [&](const google::protobuf::Empty &req) {
        return libmvm->GetMvmDevices(context, &devices, &number);
      },
      [&]() {
        memverge::libmvm::v1::GetMvmDevicesResponse res;
        for (int i = 0; i < number; i++) {
          EncodeMvmDeviceProto(devices[i], res.add_devices());
        }
        libmvm->FreeMvmDevices(devices, number);
        return res;
      },
      empty_err_out_fn, in_params, out);
}


static std::unordered_map<std::string,
                          int (*)(LibmvmInterface *libmvm, const mvmapi_context *context,
                                  const std::string &in_params, std::string *out)>
    processing_funcs = {
        {"GetMvmDevices", &GetMvmDevices},
};

static sigjmp_buf mark;
void handler(int signum) {
  void *array[10];
  char **strings;
  int size, i;

  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);
  if (strings != NULL)
  {
    MVMLOG("libmvm_bin got signal %d, printing %d stack frames.\n", signum, size);
    for (i = 0; i < size; i++) {
      MVMLOG("[frame%d]\t%s\n", i, strings[i]);
    }
  }
  free (strings);

  siglongjmp(mark, -1);
  return;
}
/**
 * The arguments are defined as:
 * $1: input pipe
 * $2: output pipe
 * $3: eventfd for task completion notification
 * $4: fd of mvsvcd_log
 * $5: log trace
 * $6: calling libmvm method name
 * $7-$16 onwards: fields for mvmapi_context
 */
int main(int argc, char *argv[]) {
  LibmvmImpl libmvm(GlobalConfig::Instance()->GetSocketPath(),
                    GlobalConfig::Instance()->GetDataDir(),
                    GlobalConfig::Instance()->GetInstallDir() + "/lib64/mvsnap", "",
                    GlobalConfig::Instance()->GetInstallDir() + "/sbin/mvmcli",
                    GlobalConfig::Instance()->GetTmpExecDir(), "", false, "");
  bool arg_check_failed = false;
  int pipe_in, pipe_out, efd;
  std::string method;
  mvmapi_context context;
  if (argc >= 5) {
    try {
      pipe_in = std::stoi(argv[1]);
      pipe_out = std::stoi(argv[2]);
      efd = std::stoi(argv[3]);
      mvmlog_fd = std::stoi(argv[4]);
      SetLibMvmLogFd(mvmlog_fd);
      log_trace = (std::stoi(argv[5]) != 0);
      SetLibMvmLogTrace(log_trace);
      method = argv[6];
      context.uid = std::stoul(argv[7]);
      strncpy(context.socket_path, argv[8], sizeof(context.socket_path) - 1);
      strncpy(context.data_dir, argv[9], sizeof(context.data_dir) - 1);
      strncpy(context.lib_dir, argv[10], sizeof(context.lib_dir) - 1);
      strncpy(context.mvsnap_log, argv[11], sizeof(context.mvsnap_log) - 1);
      strncpy(context.mvmcli_path, argv[12], sizeof(context.mvmcli_path) - 1);
      strncpy(context.tmp_exec_dir, argv[13], sizeof(context.tmp_exec_dir) - 1);
      strncpy(context.mvsnap_path, argv[14], sizeof(context.mvsnap_path) - 1);
      context.use_multicast = (std::stoi(argv[15]) != 0);
      strncpy(context.mcast_addr, argv[16], sizeof(context.mcast_addr) - 1);
    } catch (const std::exception &e) {
      fprintf(stderr, "Malformed arguments.\n");
      arg_check_failed = true;
    }
  } else {
    fprintf(stderr, "Required arguments not found.\n");
    arg_check_failed = true;
  }
  if (arg_check_failed) {
    fprintf(stderr, "Invalid arguments.\n");
    return -1;
  }
  ScopeGuard guard_pipe_in([pipe_in]() { close(pipe_in); });
  ScopeGuard guard_pipe_out([pipe_out]() { close(pipe_out); });
  ScopeGuard guard_efd([efd]() {
    /* signal completion of execution */
    uint64_t t = 1;
    write(efd, &t, sizeof(t));
    close(efd);
  });
  if (sigsetjmp(mark, 1) != 0) {
    uint64_t t = 1;
    write(efd, &t, sizeof(t));
    close(efd);
    MVMLOG("Houston, we have a problem.\n");
    exit(1);
  }
  signal(SIGSEGV, handler);
  signal(SIGILL, handler);
  signal(SIGBUS, handler);
  signal(SIGABRT, handler);
  std::string in_params;
  char buf[4096];
  while (true) {
    int r = read(pipe_in, buf, sizeof(buf));
    if (r < 0 && errno != EINTR) {
      MVMLOG("Failed reading input parameters from parent\n");
      return -1;
    } else if (r > 0) {
      in_params.insert(in_params.size(), buf, r);
    } else if (r == 0) {
      break;
    }
  }
  auto it = processing_funcs.find(method);
  MvmErrorPtr error = nullptr;
  int ret = 0;
  std::string out;
  if (it == processing_funcs.end()) {
    ret = -1;
    error =
        MvmError::CreateLibmvmError(LIBMVM_ERR_INVALID_ARGUMENT, "invalid method name");
    memverge::MvmErrorProto proto = PackMvmErrorProto(error);
    bool r = proto.SerializeToString(&out);
    assert(r);
  } else {
    ret = it->second(&libmvm, &context, in_params, &out);
  }
  size_t offset = 0;
  while (offset < out.size()) {
    int r = write(pipe_out, &out[offset], out.size() - offset);
    if (r < 0 && errno != EINTR) {
      MVMLOG("Failed writing error message to pipe %d: %m\n", pipe_out);
      ret = -1;
      break;
    } else if (r > 0) {
      offset += r;
    }
  }
  return ret;
}

