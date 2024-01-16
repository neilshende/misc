.PHONY: all clean

stage1: t1 t2 t3
	#commands for stage1
	@sleep 10
	@touch stage1
	@echo stage1 done

t1:
	# Command for t1
	@sleep 10
	@touch t1
	@echo t1 done

t2:
	# Command for t2
	@sleep 5
	@touch t2
	@echo t2 done

t3:
	# Command for t3
	@sleep 1
	@touch t3
	@echo t3 done

stage2: stage1
	# Command for stage2
	@sleep 2
	@touch stage2
	@echo stage2 done

all: stage2

clean:
	@rm -rf t1 t2 t3 stage1 stage2
