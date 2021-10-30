```(bash)
mkdir build; cd build; cmake ..; make    # build project

cd lexer
./test_lexerr                            # run lexer_tests
./lexer [files ..]                       # run lexer

cd ../parser;
./test_parser                            # run parser_tests
./lexer [files ..] | ./parser            # run parser
```
