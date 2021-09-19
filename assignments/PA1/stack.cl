class List {
    isNil() : Bool { true };
    head()  : String { { abort(); "0"; } };
    tail()  : List { { abort(); self; } };
    cons(i : String) : List {
        (new Cons).init(i, self)
    };
};

class Cons inherits List {
    car : String;
    cdr : List;
    isNil() : Bool { false };
    head()  : String { car };
    tail()  : List { cdr };
    init(i : String, rest : List) : List {
    {
        car <- i;
        cdr <- rest;
        self;
    }
    };
};

class Main inherits IO {
   input : String;
   was_terminate : Bool <- false;
   stack_machine : List <- new List;

   print_list(l : List) : Object {
      if l.isNil() then out_string("")
      else {
            out_string(l.head());
            out_string("\n");
            print_list(l.tail());
      }
      fi
   };

    run_cmd(cmd : String) : Object {
         if cmd.length() = 0 then "nop" else
         if cmd = "+" then stack_machine <- stack_machine.cons(cmd) else
         if cmd = "s" then stack_machine <- stack_machine.cons(cmd) else
         if cmd = "e" then {
            if stack_machine.isNil() then "nop" else
            if stack_machine.head() = "+" then {
                  let z : A2I <- new A2I in
                  let lhs : Int <- z.a2i(stack_machine.tail().head()) in 
                  let rhs : Int <- z.a2i(stack_machine.tail().tail().head()) in
                  stack_machine <- stack_machine.tail().tail().tail().cons(z.i2a(lhs + rhs));
            } else 
            if stack_machine.head() = "s" then {
                  let lhs : String <- stack_machine.tail().head() in 
                  let rhs : String <- stack_machine.tail().tail().head() in
                  stack_machine <- stack_machine.tail().tail().tail().cons(lhs).cons(rhs);
            } else {
                  "nop";
            }
            fi fi fi;
         } else
         if cmd = "d" then print_list(stack_machine) else
            stack_machine <- stack_machine.cons(cmd)
         fi fi fi fi fi
    };

    main(): Object {
        {
            while not was_terminate loop {
                out_string(">");
                input <- in_string();
                if input = "x" then
                    was_terminate <- true
                else
                    run_cmd(input)
                fi;
            }
            pool;
            0;
        }
    };
};
