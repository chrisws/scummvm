# main.cpp is built under eclipse

MODULE := backends/platform/bada

CXXFLAGS := -I"pch" $(CXXFLAGS) -DFORBIDDEN_SYMBOL_ALLOW_ALL

MODULE_OBJS :=  \
  audio.o       \
  application.o \
  form.o        \
  fs.o          \
  graphics.o    \
  missing.o     \
  sscanf.o      \
  system.o

MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)

$(EXECUTABLE): $(OBJS)
	rm -f $@
	arm-linux-gnueabi-ar Tru $@ $(OBJS)
