/*
 * This file is part of Chiaki.
 *
 * Chiaki is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Chiaki is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Chiaki.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <chiaki/rpcrypt.h>

#include <openssl/hmac.h>
#include <openssl/evp.h>

#include <string.h>


CHIAKI_EXPORT void chiaki_rpcrypt_bright_ambassador(uint8_t *bright, uint8_t *ambassador, const uint8_t *nonce, const uint8_t *morning)
{
	static const uint8_t echo_a[] = { 0x01, 0x49, 0x87, 0x9b, 0x65, 0x39, 0x8b, 0x39, 0x4b, 0x3a, 0x8d, 0x48, 0xc3, 0x0a, 0xef, 0x51 };
	static const uint8_t echo_b[] = { 0xe1, 0xec, 0x9c, 0x3a, 0xdd, 0xbd, 0x08, 0x85, 0xfc, 0x0e, 0x1d, 0x78, 0x90, 0x32, 0xc0, 0x04 };

	for(uint8_t i=0; i<CHIAKI_KEY_BYTES; i++)
	{
		uint8_t v = nonce[i];
		v -= i;
		v -= 0x27;
		v ^= echo_a[i];
		ambassador[i] = v;
	}

	for(uint8_t i=0; i<CHIAKI_KEY_BYTES; i++)
	{
		uint8_t v = morning[i];
		v -= i;
		v += 0x34;
		v ^= echo_b[i];
		v ^= nonce[i];
		bright[i] = v;
	}
}


CHIAKI_EXPORT void chiaki_rpcrypt_init(ChiakiRPCrypt *rpcrypt, const uint8_t *nonce, const uint8_t *morning)
{
	chiaki_rpcrypt_bright_ambassador(rpcrypt->bright, rpcrypt->ambassador, nonce, morning);
}

CHIAKI_EXPORT ChiakiErrorCode chiaki_rpcrypt_generate_iv(ChiakiRPCrypt *rpcrypt, uint8_t *iv, uint64_t counter)
{
	uint8_t hmac_key[] = { 0xac, 0x07, 0x88, 0x83, 0xc8, 0x3a, 0x1f, 0xe8, 0x11, 0x46, 0x3a, 0xf3, 0x9e, 0xe3, 0xe3, 0x77 };

	uint8_t buf[CHIAKI_KEY_BYTES + 8];
	memcpy(buf, rpcrypt->ambassador, CHIAKI_KEY_BYTES);
	buf[CHIAKI_KEY_BYTES + 0] = (uint8_t)((counter >> 0x38) & 0xff);
	buf[CHIAKI_KEY_BYTES + 1] = (uint8_t)((counter >> 0x30) & 0xff);
	buf[CHIAKI_KEY_BYTES + 2] = (uint8_t)((counter >> 0x28) & 0xff);
	buf[CHIAKI_KEY_BYTES + 3] = (uint8_t)((counter >> 0x20) & 0xff);
	buf[CHIAKI_KEY_BYTES + 4] = (uint8_t)((counter >> 0x18) & 0xff);
	buf[CHIAKI_KEY_BYTES + 5] = (uint8_t)((counter >> 0x10) & 0xff);
	buf[CHIAKI_KEY_BYTES + 6] = (uint8_t)((counter >> 0x08) & 0xff);
	buf[CHIAKI_KEY_BYTES + 7] = (uint8_t)((counter >> 0x00) & 0xff);

	uint8_t hmac[32];
	unsigned int hmac_len = 0;
	if(!HMAC(EVP_sha256(), hmac_key, CHIAKI_KEY_BYTES, buf, sizeof(buf), hmac, &hmac_len))
		return CHIAKI_ERR_UNKNOWN;

	if(hmac_len < CHIAKI_KEY_BYTES)
		return CHIAKI_ERR_UNKNOWN;

	memcpy(iv, hmac, CHIAKI_KEY_BYTES);
	return CHIAKI_ERR_SUCCESS;
}