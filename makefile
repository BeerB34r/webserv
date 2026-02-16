SRC				:=	main.cpp parse.cpp
SRCDIR			:=	src/

BIN				=	$(addprefix $(BINDIR),$(SRC:.cpp=.o))
BINDIR			:=	bin/
DEP				=	$(addprefix $(DEPDIR),$(SRC:.cpp=.d))
DEPDIR			:=	dep/
DEPFLAGS		=	-MM -MF $@ -MT $@ -MT $(BINDIR)$(addsuffix .o,$(notdir $(basename $<)))
INC				=	-I$(INCDIR)
INCDIR			:=	inc/
VPATH			=	$(SRCDIR)
CXXFLAGS		?=	-Wall -Wextra -Werror -pedantic
CPPFLAGS		=	$(INC) -std=c++20
LDFLAGS			:=
CXX				:=	c++
RM				:=	rm -fr

MAKEFLAGS		+=	-r --no-print-directory -j
.EXTRA_PREREQS	=	$(firstword $(MAKEFILE_LIST))
.DEFAULT_GOAL	:=	all
.PRECIOUS		:	$(BINDIR) $(DEPDIR)
.PHONY			:	clean fclean re all clangd

SERIOUS_NAME	=	webserv
FUNNY_NAME		=	ginxnay
NAME			=	$(SERIOUS_NAME)
ifneq ($(origin fun_allowed),undefined)
	NAME		=	$(FUNNY_NAME)
	CPPFLAGS	+=	-DFUN_ALLOWED=1
endif

-include $(DEP)

all				:	$(NAME)

$(NAME)			:	$(BIN) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(DEPDIR)%.d	:	%.cpp | $(DEPDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) $<
$(BINDIR)%.o	:	%.cpp | $(BINDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%/				:
	@mkdir -p $@

clean			:
	$(RM) $(BINDIR)
fclean			:
	$(RM) $(BINDIR)
	$(RM) $(SERIOUS_NAME) $(FUNNY_NAME) $(DEPDIR)
re				:
	+$(MAKE) fclean
	+$(MAKE) all
clangd			:
	intercept-build-14 $(MAKE) re
