#
# Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
########## Make rule for test oop750  ########


oop750: run
	

build:  $(SRC)/oop750.f90
	-$(RM) oop750.$(EXESUFFIX) core *.d *.mod FOR*.DAT FTN* ftn* fort.*
	@echo ------------------------------------ building test $@
	-$(CC) -c $(CFLAGS) $(SRC)/check.c -o check.$(OBJX)
	-$(FC) -c $(FFLAGS) $(LDFLAGS) $(SRC)/oop750.f90 -o oop750.$(OBJX)
	-$(FC) $(FFLAGS) $(LDFLAGS) oop750.$(OBJX) check.$(OBJX) $(LIBS) -o oop750.$(EXESUFFIX)


run: 
	@echo ------------------------------------ executing test oop750
	oop750.$(EXESUFFIX)

verify: ;

