import { pedersenCompressWithHashIndex } from '@aztec/barretenberg.js/crypto';
import { BarretenbergWasm } from '@aztec/barretenberg.js/wasm';
import {
  CircuitsWasm,
  KernelCircuitPublicInputs,
  PreviousKernelData,
  PrivateCallData,
  PrivateCircuitPublicInputs,
  Proof,
  SignedTxRequest,
  makeEmptyProof,
  privateKernelSimInner,
  privateKernelSimInit,
} from '@aztec/circuits.js';
import { Fr } from '@aztec/foundation/fields';
import { createDebugLogger } from '@aztec/foundation/log';

/**
 * Represents the output of the proof creation process.
 * Contains the public inputs required for the kernel circuit and the generated proof.
 */
export interface ProofOutput {
  /**
   * The public inputs required for the proof generation process.
   */
  publicInputs: KernelCircuitPublicInputs;
  /**
   * The zk-SNARK proof for the kernel execution.
   */
  proof: Proof;
}

/**
 * ProofCreator provides functionality to create and validate proofs, and retrieve
 * siloed commitments necessary for maintaining transaction privacy and security on the network.
 */
export interface ProofCreator {
  getSiloedCommitments(publicInputs: PrivateCircuitPublicInputs): Promise<Fr[]>;
  createProofInit(signedTxRequest: SignedTxRequest, privateCallData: PrivateCallData): Promise<ProofOutput>;
  createProofInner(previousKernelData: PreviousKernelData, privateCallData: PrivateCallData): Promise<ProofOutput>;
}

const OUTER_COMMITMENT = 3;

/**
 * The KernelProofCreator class is responsible for generating siloed commitments and zero-knowledge proofs
 * for private kernel circuit. It leverages Barretenberg and Circuits Wasm libraries
 * to perform cryptographic operations and proof creation. The class provides methods to compute commitments
 * based on the given public inputs and to generate proofs based on signed transaction requests, previous kernel
 * data, private call data, and a flag indicating whether it's the first iteration or not.
 */
export class KernelProofCreator {
  constructor(private log = createDebugLogger('aztec:kernel_proof_creator')) {}

  /**
   * Computes the siloed commitments for a given set of public inputs.
   *
   * @param publicInputs - The public inputs containing the contract address and new commitments to be used in generating siloed commitments.
   * @returns An array of Fr (finite field) elements representing the siloed commitments.
   */
  public async getSiloedCommitments(publicInputs: PrivateCircuitPublicInputs) {
    const bbWasm = await BarretenbergWasm.get();
    const contractAddress = publicInputs.callContext.storageContractAddress.toBuffer();
    // TODO
    // Should match `add_contract_address_to_commitment` in hash.hpp.
    // Should use a function exported from circuits.js.
    return publicInputs.newCommitments.map(commitment =>
      Fr.fromBuffer(pedersenCompressWithHashIndex(bbWasm, [contractAddress, commitment.toBuffer()], OUTER_COMMITMENT)),
    );
  }

  /**
   * Creates a proof output for a given signed transaction request and private call data for the first iteration.
   *
   * @param signedTxRequest - The signed transaction request object.
   * @param privateCallData - The private call data object.
   * @returns A Promise resolving to a ProofOutput object containing public inputs and the kernel proof.
   */
  public async createProofInit(
    signedTxRequest: SignedTxRequest,
    privateCallData: PrivateCallData,
  ): Promise<ProofOutput> {
    const wasm = await CircuitsWasm.get();
    this.log('Executing private kernel simulation init...');
    const publicInputs = await privateKernelSimInit(wasm, signedTxRequest, privateCallData);
    this.log('Skipping private kernel proving...');
    // TODO
    const proof = makeEmptyProof();
    this.log('Kernel Prover Completed!');

    return {
      publicInputs,
      proof,
    };
  }

  /**
   * Creates a proof output for a given previous kernel data and private call data for an inner iteration.
   *
   * @param previousKernelData - The previous kernel data object.
   * @param privateCallData - The private call data object.
   * @returns A Promise resolving to a ProofOutput object containing public inputs and the kernel proof.
   */
  public async createProofInner(
    previousKernelData: PreviousKernelData,
    privateCallData: PrivateCallData,
  ): Promise<ProofOutput> {
    const wasm = await CircuitsWasm.get();
    this.log('Executing private kernel simulation inner...');
    const publicInputs = await privateKernelSimInner(wasm, previousKernelData, privateCallData);
    this.log('Skipping private kernel proving...');
    // TODO
    const proof = makeEmptyProof();
    this.log('Kernel Prover Completed!');

    return {
      publicInputs,
      proof,
    };
  }
}