# 总的 Makefile，用于调用目录下各个子工程对应的 Makefile
# 注意： Linux 下编译方式：
# 1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
# 2. 下载后，解压到 /opt/jieli 目录下，保证
#   /opt/jieli/common/bin/clang 存在（注意目录层次）
# 3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#   可以通过 ulimit -n 8096 来设置一个较大的值
# 支持的目标
# make aw31n_transfer
# make aw31n_hid

.PHONY: all clean aw31n_transfer aw31n_hid clean_aw31n_transfer clean_aw31n_hid

all: aw31n_transfer aw31n_hid
	@echo +ALL DONE

clean: clean_aw31n_transfer clean_aw31n_hid
	@echo +CLEAN DONE

aw31n_transfer:
	$(MAKE) -C apps/demo/transfer/board/bd47 -f Makefile

clean_aw31n_transfer:
	$(MAKE) -C apps/demo/transfer/board/bd47 -f Makefile clean

aw31n_hid:
	$(MAKE) -C apps/demo/hid/board/bd47 -f Makefile

clean_aw31n_hid:
	$(MAKE) -C apps/demo/hid/board/bd47 -f Makefile clean
