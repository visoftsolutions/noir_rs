import { Schnorr } from '@aztec/circuits.js/barretenberg';

import { AuthPayload, AztecAddress, EntrypointPayload } from '../index.js';

/**
 * Implementation of a schnorr signature provider
 */
export class SchnorrAuthProvider {
  constructor(private signer: Schnorr, private privateKey: Buffer) {}
  authenticateTx(_payload: EntrypointPayload, _payloadHash: Buffer, _address: AztecAddress): Promise<AuthPayload> {
    const sig = this.signer.constructSignature(_payloadHash, this.privateKey);
    return Promise.resolve(sig as AuthPayload);
  }
}