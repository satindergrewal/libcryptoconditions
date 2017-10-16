
#include "asn/Condition.h"
#include "asn/Fulfillment.h"
#include "asn/PrefixFingerprintContents.h"
#include "asn/OCTET_STRING.h"
#include "include/cJSON.h"
#include "cryptoconditions.h"


int prefixVerify(CC *cond, char *msg) {
    return 1; // TODO
}


char *prefixFingerprint(CC *cond) {
    PrefixFingerprintContents_t fp;
    Condition_t *subCond = asnCondition(cond->subcondition);
    fp.subcondition = *subCond;
    free(subCond);
    fp.maxMessageLength = cond->maxMessageLength;
    //OCTET_STRING_fromBuf(&fp.prefix, cond->prefix, cond->prefixLength);
    fp.prefix =* OCTET_STRING_new_fromBuf(&asn_DEF_OCTET_STRING, cond->prefix, cond->prefixLength);
    /* Encode and hash the result */
    char out[1000^2];
    char *hash = malloc(32);
    asn_enc_rval_t rc = der_encode_to_buffer(&asn_DEF_PrefixFingerprintContents, &fp, out, 1024^2);
    if (rc.encoded == -1) {
        //panic?
    }
    crypto_hash_sha256(hash, out, rc.encoded);
    //asn_DEF_OCTET_STRING.free_struct(&asn_DEF_OCTET_STRING, &(fp.publicKey), 0);
    return hash;
}


unsigned long prefixCost(CC *cond) {
    return 1024 + cond->prefixLength + cond->maxMessageLength +
        cond->subcondition->type->getCost(cond->subcondition);
}


void prefixFfillToCC(Fulfillment_t *ffill, CC *cond) {
    cond->type = &prefixType;
    PrefixFulfillment_t *p = ffill->choice.prefixSha256;
    cond->maxMessageLength = p->maxMessageLength;
    cond->prefix = malloc(p->prefix.size);
    memcpy(cond->prefix, p->prefix.buf, p->prefix.size);
    cond->prefixLength = p->prefix.size;
    cond->subcondition = malloc(sizeof(CC));
    ffillToCC(p->subfulfillment, cond->subcondition);
}


uint32_t prefixSubtypes(CC *cond) {
    return getSubtypes(cond->subcondition) & ~(1 << prefixType.typeId);
}


CC *prefixFromJSON(cJSON *params, char *err) {
    cJSON *mml_item = cJSON_GetObjectItem(params, "maxMessageLength");
    cJSON *prefix_item = cJSON_GetObjectItem(params, "prefix");
    cJSON *subcond_item = cJSON_GetObjectItem(params, "subfulfillment");

    if (!cJSON_IsNumber(mml_item)) {
        strcpy(err, "maxMessageLength must be a number");
        return NULL;
    }

    if (!cJSON_IsString(prefix_item)) {
        strcpy(err, "prefix must be a string");
        return NULL;
    }

    if (!cJSON_IsObject(subcond_item)) {
        strcpy(err, "subfulfillment must be an oject");
        return NULL;
    }

    CC *cond = malloc(sizeof(CC));
    cond->type = &prefixType;
    cond->maxMessageLength = (unsigned long) mml_item->valuedouble;
    CC *sub = conditionFromJSON(subcond_item, err);
    if (NULL == sub) {
        return NULL;
    }
    cond->subcondition = sub;

    cond->prefix = base64_decode(prefix_item->valuestring, // TODO: verify
            strlen(prefix_item->valuestring), &cond->prefixLength);
    cJSON_free(mml_item);
    cJSON_free(prefix_item);
    cJSON_free(subcond_item);
    return cond;
}


struct CCType prefixType = { 1, "prefix-sha-256", Condition_PR_prefixSha256, 1, &prefixVerify, &prefixFingerprint, &prefixCost, &prefixSubtypes, &prefixFromJSON, &prefixFfillToCC };
