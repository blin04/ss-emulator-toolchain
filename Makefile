LEXER_SRC = misc/lex.yy.c
PARSER_SRC = misc/parser.tab.c
PARSER_HDR = misc/parser.tab.h
ASM_SRC = src/asm.cpp

SOURCES = $(ASM_SRC) $(PARSER_SRC) $(LEXER_SRC)
OUTPUT = build/asembler


asembler: $(SOURCES)
	g++ -I misc -I inc $(SOURCES) -o $(OUTPUT)

lexer: $(LEXER_SRC)

$(LEXER_SRC): misc/lexer.l
	flex -o $@ $<

parser: $(PARSER_SRC) $(PARSER_HDR)
 
$(PARSER_SRC) $(PARSER_HDR): misc/parser.y
	bison -d -o $(PARSER_SRC) $<

clean:
	rm -f $(LEXER_SRC) $(PARSER_SRC) $(PARSER_HDR) $(OUTPUT)