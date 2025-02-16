#include "rust_bind.hpp"
#include "../pedersen_hash/pedersen.hpp"
#include "barretenberg/common/serialize.hpp"
#include "pedersen.hpp"

extern "C" {

using namespace barretenberg;

WASM_EXPORT const char* rust_pedersen_commit(fr::vec_in_buf inputs_buffer, fr::out_buf output)
{
    try {
        std::vector<grumpkin::fq> to_commit;
        read(inputs_buffer, to_commit);
        grumpkin::g1::affine_element pedersen_commitment = crypto::pedersen_commitment::commit_native(to_commit);

        serialize::write(output, pedersen_commitment);
        return nullptr;
    } catch (const std::exception& e) {
        return e.what(); // return the exception message
    }
}
}