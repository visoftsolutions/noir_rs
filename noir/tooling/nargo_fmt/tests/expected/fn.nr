fn main(x: pub u8, y: u8) {}

fn main(x: pub u8, y: u8) -> pub Field {}

fn main<A, B>(x: A, y: B) -> pub Field where A: Eq, B: Eq {}

fn main()
// hello
{}

fn main(
    // hello
) {}

fn main(
    // hello
    unit: ()
) {}

fn main<T>() where T: Eq {}

fn main(
    tape: [Field; TAPE_LEN],
    initial_registers: [Field; REGISTER_COUNT],
    initial_memory: [Field; MEM_COUNT],
    initial_program_counter: Field,
    initial_call_stack: [Field; MAX_CALL_STACK],
    initial_call_stack_pointer: u64
) -> pub ExecutionResult {}

fn apply_binary_field_op<N>(
    lhs: RegisterIndex,
    rhs: RegisterIndex,
    result: RegisterIndex,
    op: u8,
    registers: &mut Registers<N>
) -> bool {}

fn main() -> distinct pub [Field;2] {}

fn main(
    message: [u8; 10],
    message_field: Field,
    pub_key_x: Field,
    pub_key_y: Field,
    signature: [u8; 64]
) {}
