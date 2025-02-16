#pragma once
#include "barretenberg/polynomials/polynomial.hpp"
#include "barretenberg/proof_system/arithmetization/arithmetization.hpp"
#include "barretenberg/proof_system/op_queue/ecc_op_queue.hpp"
#include "barretenberg/proof_system/plookup_tables/plookup_tables.hpp"
#include "barretenberg/proof_system/plookup_tables/types.hpp"
#include "barretenberg/proof_system/types/merkle_hash_type.hpp"
#include "barretenberg/proof_system/types/pedersen_commitment_type.hpp"
#include "ultra_circuit_builder.hpp"
#include <optional>

namespace proof_system {

using namespace barretenberg;

template <typename FF> class GoblinUltraCircuitBuilder_ : public UltraCircuitBuilder_<arithmetization::UltraHonk<FF>> {
  public:
    static constexpr std::string_view NAME_STRING = "GoblinUltraArithmetization";
    static constexpr CircuitType CIRCUIT_TYPE = CircuitType::ULTRA;
    static constexpr size_t DEFAULT_NON_NATIVE_FIELD_LIMB_BITS =
        UltraCircuitBuilder_<arithmetization::UltraHonk<FF>>::DEFAULT_NON_NATIVE_FIELD_LIMB_BITS;

    size_t num_ecc_op_gates = 0; // number of ecc op "gates" (rows); these are placed at the start of the circuit

    // Stores record of ecc operations and performs corresponding native operations internally
    std::shared_ptr<ECCOpQueue> op_queue;

    // Indices for constant variables corresponding to ECCOpQueue op codes
    uint32_t null_op_idx;
    uint32_t add_accum_op_idx;
    uint32_t mul_accum_op_idx;
    uint32_t equality_op_idx;

    using WireVector = std::vector<uint32_t, ContainerSlabAllocator<uint32_t>>;
    using SelectorVector = std::vector<FF, ContainerSlabAllocator<FF>>;

    // Wires storing ecc op queue data; values are indices into the variables array
    std::array<WireVector, arithmetization::UltraHonk<FF>::NUM_WIRES> ecc_op_wires;

    WireVector& ecc_op_wire_1() { return std::get<0>(ecc_op_wires); };
    WireVector& ecc_op_wire_2() { return std::get<1>(ecc_op_wires); };
    WireVector& ecc_op_wire_3() { return std::get<2>(ecc_op_wires); };
    WireVector& ecc_op_wire_4() { return std::get<3>(ecc_op_wires); };

    const WireVector& ecc_op_wire_1() const { return std::get<0>(ecc_op_wires); };
    const WireVector& ecc_op_wire_2() const { return std::get<1>(ecc_op_wires); };
    const WireVector& ecc_op_wire_3() const { return std::get<2>(ecc_op_wires); };
    const WireVector& ecc_op_wire_4() const { return std::get<3>(ecc_op_wires); };

    SelectorVector& q_busread() { return this->selectors.q_busread(); };
    SelectorVector& q_poseidon2_external() { return this->selectors.q_poseidon2_external(); };
    SelectorVector& q_poseidon2_internal() { return this->selectors.q_poseidon2_internal(); };

    const SelectorVector& q_busread() const { return this->selectors.q_busread(); };
    const SelectorVector& q_poseidon2_external() const { return this->selectors.q_poseidon2_external(); };
    const SelectorVector& q_poseidon2_internal() const { return this->selectors.q_poseidon2_internal(); };

    // DataBus call/return data arrays
    std::vector<uint32_t> public_calldata;
    std::vector<uint32_t> calldata_read_counts;
    std::vector<uint32_t> public_return_data;

    // Functions for adding ECC op queue "gates"
    ecc_op_tuple queue_ecc_add_accum(const g1::affine_element& point);
    ecc_op_tuple queue_ecc_mul_accum(const g1::affine_element& point, const FF& scalar);
    ecc_op_tuple queue_ecc_eq();

  private:
    void populate_ecc_op_wires(const ecc_op_tuple& in);
    ecc_op_tuple decompose_ecc_operands(uint32_t op, const g1::affine_element& point, const FF& scalar = FF::zero());

  public:
    GoblinUltraCircuitBuilder_(const size_t size_hint = 0,
                               std::shared_ptr<ECCOpQueue> op_queue_in = std::make_shared<ECCOpQueue>())
        : UltraCircuitBuilder_<arithmetization::UltraHonk<FF>>(size_hint)
        , op_queue(op_queue_in)
    {
        // Set indices to constants corresponding to Goblin ECC op codes
        null_op_idx = this->zero_idx;
        add_accum_op_idx = this->put_constant_variable(FF(EccOpCode::ADD_ACCUM));
        mul_accum_op_idx = this->put_constant_variable(FF(EccOpCode::MUL_ACCUM));
        equality_op_idx = this->put_constant_variable(FF(EccOpCode::EQUALITY));
    };
    GoblinUltraCircuitBuilder_(std::shared_ptr<ECCOpQueue> op_queue_in)
        : GoblinUltraCircuitBuilder_(0, op_queue_in)
    {}

    void finalize_circuit();
    void add_gates_to_ensure_all_polys_are_non_zero();

    size_t get_num_constant_gates() const override { return 0; }

    /**
     * @brief Get the final number of gates in a circuit, which consists of the sum of:
     * 1) Current number number of actual gates
     * 2) Number of public inputs, as we'll need to add a gate for each of them
     * 3) Number of Rom array-associated gates
     * 4) Number of range-list associated gates
     * 5) Number of non-native field multiplication gates.
     *
     * @return size_t
     */
    size_t get_num_gates() const override
    {
        auto num_ultra_gates = UltraCircuitBuilder_<arithmetization::UltraHonk<FF>>::get_num_gates();
        return num_ultra_gates + num_ecc_op_gates;
    }

    /**x
     * @brief Print the number and composition of gates in the circuit
     *
     */
    virtual void print_num_gates() const override
    {
        size_t count = 0;
        size_t rangecount = 0;
        size_t romcount = 0;
        size_t ramcount = 0;
        size_t nnfcount = 0;
        UltraCircuitBuilder_<arithmetization::UltraHonk<FF>>::get_num_gates_split_into_components(
            count, rangecount, romcount, ramcount, nnfcount);

        size_t total = count + romcount + ramcount + rangecount + num_ecc_op_gates;
        std::cout << "gates = " << total << " (arith " << count << ", rom " << romcount << ", ram " << ramcount
                  << ", range " << rangecount << ", non native field gates " << nnfcount << ", goblin ecc op gates "
                  << num_ecc_op_gates << "), pubinp = " << this->public_inputs.size() << std::endl;
    }

    /**
     * Make a witness variable a member of the public calldata.
     *
     * @param witness_index The index of the witness.
     * */
    void set_public_calldata(const uint32_t witness_index)
    {
        for (const uint32_t calldata : public_calldata) {
            if (calldata == witness_index) {
                if (!this->failed()) {
                    this->failure("Attempted to redundantly set a public calldata!");
                }
                return;
            }
        }
        public_calldata.emplace_back(witness_index);
    }
    void create_poseidon2_external_gate(const poseidon2_external_gate_<FF>& in);
    void create_poseidon2_internal_gate(const poseidon2_internal_gate_<FF>& in);

    FF compute_poseidon2_external_identity(FF q_poseidon2_external_value,
                                           FF q_1_value,
                                           FF q_2_value,
                                           FF q_3_value,
                                           FF q_4_value,
                                           FF w_1_value,
                                           FF w_2_value,
                                           FF w_3_value,
                                           FF w_4_value,
                                           FF w_1_shifted_value,
                                           FF w_2_shifted_value,
                                           FF w_3_shifted_value,
                                           FF w_4_shifted_value,
                                           FF alpha_base,
                                           FF alpha) const;

    FF compute_poseidon2_internal_identity(FF q_poseidon2_internal_value,
                                           FF q_1_value,
                                           FF w_1_value,
                                           FF w_2_value,
                                           FF w_3_value,
                                           FF w_4_value,
                                           FF w_1_shifted_value,
                                           FF w_2_shifted_value,
                                           FF w_3_shifted_value,
                                           FF w_4_shifted_value,
                                           FF alpha_base,
                                           FF alpha) const;

    bool check_circuit();
};
extern template class GoblinUltraCircuitBuilder_<barretenberg::fr>;
using GoblinUltraCircuitBuilder = GoblinUltraCircuitBuilder_<barretenberg::fr>;
} // namespace proof_system
