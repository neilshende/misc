/**
 * mx_aio.c
 * (C) 2015 Maxta
 *
 * Synthesis of AIO library calls by leveraging Taskq framework.
 */
#include <sys/zfs_context.h>
#include <sys/fm/fs/zfs.h>
#include <sys/spa.h>
#include <sys/txg.h>
#include <sys/spa_impl.h>
#include <sys/vdev_impl.h>
#include <sys/zio_impl.h>
#include <sys/zio_compress.h>
#include <sys/zio_checksum.h>
#include <sys/dmu_objset.h>
#include <sys/arc.h>
#include <sys/ddt.h>
#include <syslog.h>
#ifdef MX_AIO
extern int vdev_file_get_fd_for_zio(zio_t *zio);
/*
 * TODO This shouldn't exist. We should compute pool size as 
 * 	(Number of devices in the pool) * (Max qdepth allowed by the sched)
 */
#define AIO_TASKQ_THREAD_POOL_SIZE	32
void
mx_aio_init(spa_t *spa, taskq_t **taskq)
{
	*taskq = taskq_create(NULL, AIO_TASKQ_THREAD_POOL_SIZE, maxclsyspri,
			      1, AIO_TASKQ_THREAD_POOL_SIZE, TASKQ_PREPOPULATE);
	syslog(LOG_NOTICE, "AIO taskq for pool %s size %d",
	       spa->spa_name, AIO_TASKQ_THREAD_POOL_SIZE);
}
void
mx_aio_fini(spa_t *spa, zio_aio_ctx_t *ctx)
{
	syslog(LOG_NOTICE, "AIO taskq destroy start, pool %s", spa->spa_name);
	taskq_destroy(ctx->aio_taskq);
	syslog(LOG_NOTICE, "AIO taskq destroy done, pool %s", spa->spa_name);
}
static void
mx_sync_read_write(zio_t *zio)
{
	ssize_t		total_xfer = 0;
	int             error = 0;
	int		fd;
	VERIFY(zio->io_type == ZIO_TYPE_READ ||
	       zio->io_type == ZIO_TYPE_WRITE);
	VERIFY(zio->io_size > 0);
	fd = vdev_file_get_fd_for_zio(zio);
	// TODO Any way to track delay between enqueue/dequeue?
	zio->phys_io_start_ms = zfs_cur_time_ms;
	do {
		ssize_t 	size;
		if (zio->io_type == ZIO_TYPE_READ) {
			size = pread(fd,
				      ((uint8_t*)zio->io_data) + total_xfer,
				      zio->io_size - (uint64_t)total_xfer,
				      zio->io_offset + (uint64_t)total_xfer);
		} else {
			size = pwrite(fd,
				      ((uint8_t*)zio->io_data) + total_xfer,
				      zio->io_size - (uint64_t)total_xfer,
				      zio->io_offset + (uint64_t)total_xfer);
		}
		if (size <= 0) {
			break;
		}
		total_xfer += size;
	} while (total_xfer < zio->io_size);
	VERIFY(total_xfer <= zio->io_size);
	if (total_xfer == zio->io_size) {
		zio->io_error = 0;
	} else {
		// IO failed
		zio->io_error = EIO;
		// See man page for pread/pwrite
		VERIFY(total_xfer == EOF ||
		       total_xfer == -1  ||
		       total_xfer == 0);
		syslog(LOG_WARNING,
		       "IO failure for pool %s device %s type %s size %ld "
		       "returned size %ld offset %ld errno %d",
		       zio->io_spa->spa_name,
		       zio->io_vd->vdev_path,
		       zio->io_type == ZIO_TYPE_READ ? "READ" : "WRITE",
		       zio->io_size,
		       total_xfer,
		       zio->io_offset,
		       errno);
	}
	zio_track_latency_spikes(zio);
	zio_interrupt(zio);
}
void
mx_aio_submit(zio_aio_ctx_t *ctx, zio_t *zio)
{
	taskq_dispatch(ctx->aio_taskq, (task_func_t *)mx_sync_read_write,
		       zio, TQ_SLEEP);
}
#endif // MX_AIO
