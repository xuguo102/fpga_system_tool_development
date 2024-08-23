APP = fpgapcieapp

# Add any other object files to this list below
APP_OBJS = fpgapcieapp.o

INCLUDES = -I /home/matthew/vpk120_petalinux/xilinx-vpk120-2023.2/project-spec/meta-user/recipes-apps/fpgapcieapp/files

all: build

build: $(APP)

$(APP): $(APP_OBJS)
	$(CC) $(INCLUDES) -o $@ $(APP_OBJS) $(LDFLAGS) $(LDLIBS)
clean:
	rm -f $(APP) *.o

