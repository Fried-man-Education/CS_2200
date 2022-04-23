# Authored by Christopher Tam for Georgia Tech's CS 2200
# Revised by Tanner Muldoon

CIRCUIT = LC-22a.sim
MICROCODE = microcode.xlsx
ASSEMBLY = prj2.s
SUBMIT_FILES = $(CIRCUIT) $(MICROCODE) $(ASSEMBLY)
SUBMISSION_NAME = project2-interrupts

all: ;

clean: 
	rm -f $(MICROCODE) $(ASSEMBLY)
	rm -f $(SUBMISSION_NAME).zip

submit:
	cp microcode/microcode.xlsx .
	cp assembly/prj2.s .
	@(zip $(SUBMISSION_NAME).zip $(SUBMIT_FILES) && \
	echo "Created submission archive $$(tput bold)$(SUBMISSION_NAME).zip$$(tput sgr0).")
	rm $(MICROCODE) $(ASSEMBLY)