# main.cpp is built under eclipse

MODULE := backends/platform/bada

CXXFLAGS := $(CXXFLAGS) -DFORBIDDEN_SYMBOL_ALLOW_ALL -w -D_DEBUG

MODULE_OBJS :=  \
  fs.o          \
  audio.o       \
  form.o        \
  graphics.o    \
  missing.o     \
  sscanf.o      \
  system.o

MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)

$(EXECUTABLE): $(OBJS)
	rm -f $@
	ar Tru $@ $(OBJS)
