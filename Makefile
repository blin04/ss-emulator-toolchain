LEXER_SRC = misc/lex.yy.c
PARSER_SRC = misc/parser.tab.c
PARSER_HDR = misc/parser.tab.h

ASM_HEADERS = inc/directives.hpp inc/instruction.hpp inc/objfile.hpp inc/section.hpp inc/symtab.hpp inc/interface.h inc/line.hpp
ASM_SOURCES = src/asm_main.cpp src/directives.cpp src/instruction.cpp src/objfile.cpp src/section.cpp src/symtab.cpp src/interface.cpp $(PARSER_SRC) $(LEXER_SRC)
OUTPUT = build/asembler

LINKER_HEADERS = inc/linker.hpp inc/linkfile.hpp inc/linksymtab.hpp inc/linksection.hpp inc/objreader.hpp
LINKER_SOURCES = src/linker_main.cpp src/linker.cpp src/linksymtab.cpp src/objreader.cpp
LINKER_OUTPUT = build/linker

.DEFAULT_GLOBAL = all

all: asembler linker

asembler: $(ASM_SOURCES)
	g++ -I misc -I inc $(ASM_SOURCES) -o $(OUTPUT)

asembler-debug: $(ASM_SOURCES)
	g++ -g -I misc -I inc $(ASM_SOURCES) -o $(OUTPUT)

linker: $(LINKER_SOURCES)
	g++ -I misc -I inc $(LINKER_SOURCES) -o $(LINKER_OUTPUT)

linker-debug: $(LINKER_SOURCES)
	g++ -g -I misc -I inc $(LINKER_SOURCES) -o $(LINKER_OUTPUT)

lexer: $(LEXER_SRC)

$(LEXER_SRC): misc/lexer.l
	flex -o $@ $<

parser: $(PARSER_SRC) $(PARSER_HDR)
 
$(PARSER_SRC) $(PARSER_HDR): misc/parser.y
	bison -d -o $(PARSER_SRC) $<

clean:
	rm -f $(LEXER_SRC) $(PARSER_SRC) $(PARSER_HDR) $(OUTPUT) $(LINKER_OUTPUT)