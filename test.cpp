#include<openssl/evp.h>
#include<openssl/sha.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <sys/types.h> 

#define KEYSIZE 32
typedef struct {
	uint8_t PublicKey[KEYSIZE];
	uint8_t PrivateKey[KEYSIZE];
}BoxKeys;

BoxKeys getKeyPair(void){
	size_t lenpub = KEYSIZE;
	size_t lenpriv = KEYSIZE;
	BoxKeys keys;
	EVP_PKEY_CTX * Ctx;
	EVP_PKEY * Pkey=NULL;	
	Ctx = EVP_PKEY_CTX_new_id (NID_X25519, NULL);
	EVP_PKEY_keygen_init(Ctx);
 	EVP_PKEY_keygen(Ctx, &Pkey);
	EVP_PKEY_CTX_free(Ctx);
	EVP_PKEY_get_raw_public_key (Pkey, keys.PublicKey, &lenpub);
	EVP_PKEY_get_raw_private_key (Pkey, keys.PrivateKey, &lenpriv);
	EVP_PKEY_free(Pkey);
	return keys;
}

//		unsigned char hash[SHA512_DIGEST_LENGTH];
void getSHA512(void* data, unsigned char * hash){
		SHA512_CTX sha512;
		SHA512_Init(&sha512);
		SHA512_Update(&sha512, data, KEYSIZE);
		SHA512_Final(hash, &sha512);
}
/*
  	SigningPublicKey: 1895d418045e5b08f1b1eaa331793b97a4099171ba56396d952d6915498f2c0f
        inet6 201:d98b:62d3:c0a6:b49:49ee:d950:13d  prefixlen 7  scopeid 0x0<global>
        inet6 fe80::e449:d4c0:35fb:4bc5  prefixlen 64  scopeid 0x20<link>
<Mercury> func GetNodeID(pub *BoxPubKey) *NodeID {
<Mercury>  h := sha512.Sum512(pub[:])
<Mercury>  return (*NodeID)(&h)
<Mercury> }
https://github.com/yggdrasil-network/yggdrasil-go/blob/78b5f88e4bb734d0dd6a138ff08d34ca39dcaea3/contrib/ansible/genkeys.go#L94
	https://github.com/yggdrasil-network/yggdrasil-go/blob/master/src/address/address.go#L52
https://github.com/yggdrasil-network/yggdrasil-go/blob/1fbab17b376bb8f4ee7026dded7461276681056f/src/tuntap/tun.go#L159

*/

void convertSHA512ToSum(unsigned char hash[SHA512_DIGEST_LENGTH], char outputBuffer[128]){
    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
}

//правильно ли? I think is not coorect but from -> 
/*
https://github.com/yggdrasil-network/yggdrasil-go/blob/c3f8db699138a08278017634d3ec0057db2b253c/src/crypto/crypto.go#L85
getNodeID
-> https://github.com/yggdrasil-network/yggdrasil-go/blob/1fbab17b376bb8f4ee7026dded7461276681056f/src/address/address.go#L52
<Mercury>   EncryptionPublicKey: 6ae6ed2c9554ea2252df8ec5f510cd1beb60f6ef05d94f90bdbbb5061440bb75 => is our key
<Mercury> is our ipv6
<Mercury> GuruRandomAscension_, зачем
*/


/*
// BoxPubKey is a NaCl-like "box" public key (curve25519+xsalsa20+poly1305).

type BoxPubKey [BoxPubKeyLen]byte

//info: https://github.com/yggdrasil-network/yggdrasil-go/blob/master/src/crypto/crypto.go#L185
//https://github.com/golang/crypto/blob/master/nacl/box/box.go#L59
//LOL REALLY?
//XSALSA20?!
ORIGINAL CODE:
// AddrForNodeID takes a *NodeID as an argument and returns an *Address.
// This address begins with the contents of GetPrefix(), with the last bit set to 0 to indicate an address.
// The following 8 bits are set to the number of leading 1 bits in the NodeID.
// The NodeID, excluding the leading 1 bits and the first leading 0 bit, is truncated to the appropriate length and makes up the remainder of the address.
func AddrForNodeID(nid *crypto.NodeID) *Address {
	// 128 bit address
	// Begins with prefix
	// Next bit is a 0
	// Next 7 bits, interpreted as a uint, are # of leading 1s in the NodeID
	// Leading 1s and first leading 0 of the NodeID are truncated off
	// The rest is appended to the IPv6 address (truncated to 128 bits total)
	var addr Address
	var temp []byte
	done := false
	ones := byte(0)
	bits := byte(0)
	nBits := 0
	for idx := 0; idx < 8*len(nid); idx++ {
		bit := (nid[idx/8] & (0x80 >> byte(idx%8))) >> byte(7-(idx%8))
		if !done && bit != 0 {
			ones++
			continue
		}
		if !done && bit == 0 {
			done = true
			continue // FIXME? this assumes that ones <= 127, probably only worth changing by using a variable length uint64, but that would require changes to the addressing scheme, and I'm not sure ones > 127 is realistic
		}
		bits = (bits << 1) | bit
		nBits++
		if nBits == 8 {
			nBits = 0
			temp = append(temp, bits)
		}
	}
	prefix := GetPrefix()
	copy(addr[:], prefix[:])
	addr[len(prefix)] = ones
	copy(addr[len(prefix)+1:], temp)
	return &addr
}


*/
char * convertSHA512ToIPv6(unsigned char h[SHA512_DIGEST_LENGTH]){
		char hash[128];
		convertSHA512ToSum(h, hash);

		char byte;
		bool done;
		char ones;
		int nBits=0;
		char temp[16];
		int i=0;
		int z=0;
		for (auto idx = 0; idx < sizeof(hash); idx++) {
			char bit = (hash[idx/8] & (0x80 >> (unsigned char)(idx%8))) >> (unsigned char)(7-(idx%8));
			if (!done && bit != 0) {
				ones++;
				continue;
			}
			if (!done && bit == 0) {
				done = true;
				continue; // FIXME? this assumes that ones <= 127, probably only worth changing by using a variable length uint64, but that would require changes to the addressing scheme, and I'm not sure ones > 127 is realistic
			}
			byte = (byte << 1) | bit;
			nBits++;
			if (nBits == 8) {
				nBits = 0;
				temp[i++]=(byte);
			}
		}

		
		struct in6_addr tmpAdr;
		for(int i =0; i < 16; i++)
			tmpAdr.s6_addr[i]=temp[i];
		char * addr = (char*)calloc(INET6_ADDRSTRLEN, sizeof(char));
		inet_ntop(AF_INET6, &tmpAdr, addr, INET6_ADDRSTRLEN);
		//std::cout << "adress: " << addr  << std::endl;
		return addr;
}	
int miner(void)
{
	for(;;)
	{

	// x25519 -----------------------


		auto myKeys = getKeyPair();
		puts("Public: ");
		for(int i = 0; i < 32; ++i)
		{
        		printf("%02x", myKeys.PublicKey[i]);// two byte 
		}
		puts("\nPrivate: ");
		for(int i = 0; i < 32; ++i)
		{
        		printf("%02x", myKeys.PrivateKey[i]);// two byte 
		}
		puts("");

// sha512 --------------------------------
		puts("SHA512:");
		unsigned char hash[SHA512_DIGEST_LENGTH];
		getSHA512(myKeys.PublicKey, hash);
		for(int i = 0; i < SHA512_DIGEST_LENGTH; i++)
		{
        		printf("%02x", hash[i]);// two byte 
		}
		puts("");
		puts("IPv6:");
		auto ipv6=convertSHA512ToIPv6(hash);
		printf("%s\n", ipv6);
		free(ipv6);
		puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	}
}

int main(void){
	miner();
}
