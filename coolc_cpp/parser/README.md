```(bash)
cd ..; mkdir build; cd build; cmake ..; make    # build project
cd parser;
./test_parser                                   # run tests
./lexer [files ..] | ./parser                   # run parser
```
