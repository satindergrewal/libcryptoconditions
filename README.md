# libcryptoconditions [![Build Status](https://travis-ci.org/libscott/libcryptoconditions.svg?branch=master)](https://travis-ci.org/libscott/libcryptoconditions)

Interledger Crypto-Conditions in C, targeting spec [draft-thomas-crypto-conditions-03](https://tools.ietf.org/html/draft-thomas-crypto-conditions-03).

Features shared object and easy to use JSON api, as well as a command line interface written in Python.

## Quickstart

```shell
git clone --recursive https://github.com/libscott/libcryptoconditions
cd libcryptoconditions
./autogen.sh
./configure
make
./cryptoconditions.py --help
```

## Status

JSON interface not particularly safe. The rest is getting there.

## Embedding

For the binary interface, see [cryptoconditions.h](./include/cryptoconditions.h).

To embed in other languages, the easiest way may be to call the JSON RPC method via FFI. This is how it looks in Python:

```python
import json
from ctypes import *

so = cdll.LoadLibrary('.libs/libcryptoconditions.so')
so.jsonRPC.restype = c_char_p

def call_cryptoconditions_rpc(method, params):
    out = so.jsonRPC(json.dumps({
        'method': method,
        'params': params,
    }))
    return json.loads(out)
```

## JSON methods

### encodeCondition

Encode a JSON condition to a base64 binary string

```shell
cryptoconditions encodeCondition '{
    "type": "ed25519-sha-256",
    "publicKey": "11qYAYKxCrfVS_7TyWQHOg7hcvPapiMlrwIaaPcHURo"
}'
{
    "bin": "pCeAIHmSOauo_E_36r-8TETmnovf7ZkzJOEu1keSq-KJzx1fgQMCAAA", 
    "uri": "ni:///sha-256;eZI5q6j8T_fqv7xMROaei9_tmTMk4S7WR5Kr4onPHV8?fpt=ed25519-sha-256&cost=131072"
}
```

### decodeCondition

Decode a binary condition

```shell
cryptoconditions decodeCondition '{
    "bin": "pCeAIHmSOauo_E_36r-8TETmnovf7ZkzJOEu1keSq-KJzx1fgQMCAAA"
}'
{
    "bin": "pCeAIHmSOauo_E_36r-8TETmnovf7ZkzJOEu1keSq-KJzx1fgQMCAAA", 
    "uri": "ni:///sha-256;eZI5q6j8T_fqv7xMROaei9_tmTMk4S7WR5Kr4onPHV8?fpt=ed25519-sha-256&cost=131072"
}
```

### encodeFulfillment

Encode a JSON condition to a binary fulfillment. The condition must be fulfilled, that is,
it needs to have signatures present.

```shell
cryptoconditions encodeFulfillment '{
{
    "type": "ed25519-sha-256",
    "publicKey": "E0x0Ws4GhWhO_zBoUyaLbuqCz6hDdq11Ft1Dgbe9y9k",
    "signature": "jcuovSRpHwqiC781KzSM1Jd0Qtyfge0cMGttUdLOVdjJlSBFLTtgpinASOaJpd-VGjhSGWkp1hPWuMAAZq6pAg"
}'
{
    "fulfillment": "pGSAIBNMdFrOBoVoTv8waFMmi27qgs-oQ3atdRbdQ4G3vcvZgUCNy6i9JGkfCqILvzUrNIzUl3RC3J-B7Rwwa21R0s5V2MmVIEUtO2CmKcBI5oml35UaOFIZaSnWE9a4wABmrqkC"
}

```

### decodeFulfillment

Decode a binary fulfillment

```shell
cryptoconditions decodeFulfillment '{
    "fulfillment": "pGSAINdamAGCsQq31Uv-08lkBzoO4XLz2qYjJa8CGmj3B1EagUDlVkMAw2CscpCG4syAboKKhId_Hrjl2XTYc-BlIkkBVV-4ghWQozusxh45cBz5tGvSW_XwWVu-JGVRQUOOehAL"
}'
{
    "bin": "pCeAIHmSOauo_E_36r-8TETmnovf7ZkzJOEu1keSq-KJzx1fgQMCAAA", 
    "uri": "ni:///sha-256;eZI5q6j8T_fqv7xMROaei9_tmTMk4S7WR5Kr4onPHV8?fpt=ed25519-sha-256&cost=131072"
}
```

### verifyFulfillment

Verify a fulfillment against a message and a condition URL

```shell
cryptoconditions verifyFulfillment '{
    "message": "",
    "fulfillment": "pGSAINdamAGCsQq31Uv-08lkBzoO4XLz2qYjJa8CGmj3B1EagUDlVkMAw2CscpCG4syAboKKhId_Hrjl2XTYc-BlIkkBVV-4ghWQozusxh45cBz5tGvSW_XwWVu-JGVRQUOOehAL",
    "condition": "pCeAIHmSOauo_E_36r-8TETmnovf7ZkzJOEu1keSq-KJzx1fgQMCAAA"
}
{
    "valid": true
}
```

### signTreeEd25519

Sign all ed25519 nodes in a condition tree

```shell
cryptoconditions signTreeEd25519 '{
    "condition": {
        "type": "ed25519-sha-256",
        "publicKey": "E0x0Ws4GhWhO_zBoUyaLbuqCz6hDdq11Ft1Dgbe9y9k",
    },
    "privateKey": "11qYAYKxCrfVS_7TyWQHOg7hcvPapiMlrwIaaPcHURo",
    "message": "",
}'
{
    "num_signed": 1,
    "condition": {
        "type": "ed25519-sha-256",
        "publicKey": "E0x0Ws4GhWhO_zBoUyaLbuqCz6hDdq11Ft1Dgbe9y9k",
        "signature": "jcuovSRpHwqiC781KzSM1Jd0Qtyfge0cMGttUdLOVdjJlSBFLTtgpinASOaJpd-VGjhSGWkp1hPWuMAAZq6pAg"
    }
}
```
