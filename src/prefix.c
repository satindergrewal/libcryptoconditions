
#include "asn/Condition.h"
#include "asn/Fulfillment.h"
#include "asn/PrefixFingerprintContents.h"
#include "asn/OCTET_STRING.h"
#include "include/cJSON.h"
#include "cryptoconditions.h"


struct CCType cc_prefixType;


static int prefixVisitChildren(CC *cond, CCVisitor visitor) {
    size_t prefixedLength = cond->prefixLength + visitor.msgLength;
    char *prefixed = malloc(prefixedLength);
    memcpy(prefixed, cond->prefix, cond->prefixLength);
    memcpy(prefixed + cond->prefixLength, visitor.msg, visitor.msgLength);
    visitor.msg = prefixed;
    visitor.msgLength = prefixedLength;
    int res = cc_visit(cond->subcondition, visitor);
    free(prefixed);
    return res;
}


static char *prefixFingerprint(CC *cond) {
    PrefixFingerprintContents_t *fp = calloc(1, sizeof(PrefixFingerprintContents_t));
    asnCondition(cond->subcondition, &fp->subcondition); // TODO: check asnCondition for safety
    fp->maxMessageLength = cond->maxMessageLength;
    fp->prefix.buf = calloc(1, cond->prefixLength);
    memcpy(fp->prefix.buf, cond->prefix, cond->prefixLength);
    fp->prefix.size = cond->prefixLength;
    return hashFingerprintContents(&asn_DEF_PrefixFingerprintContents, fp);
}


static unsigned long prefixCost(CC *cond) {
    return 1024 + cond->prefixLength + cond->maxMessageLength +
        cond->subcondition->type->getCost(cond->subcondition);
}


static CC *prefixFromFulfillment(Fulfillment_t *ffill) {
    PrefixFulfillment_t *p = ffill->choice.prefixSha256;
    CC *sub = fulfillmentToCC(p->subfulfillment);
    if (!sub) return 0;
    CC *cond = calloc(1, sizeof(CC));
    cond->type = &cc_prefixType;
    cond->maxMessageLength = p->maxMessageLength;
    cond->prefix = calloc(1, p->prefix.size);
    memcpy(cond->prefix, p->prefix.buf, p->prefix.size);
    cond->prefixLength = p->prefix.size;
    cond->subcondition = sub;
    return cond;
}


static Fulfillment_t *prefixToFulfillment(CC *cond) {
    Fulfillment_t *ffill = asnFulfillmentNew(cond->subcondition);
    if (!ffill) {
        return NULL;
    }
    PrefixFulfillment_t *pf = calloc(1, sizeof(PrefixFulfillment_t));
    OCTET_STRING_fromBuf(&pf->prefix, cond->prefix, cond->prefixLength);
    pf->maxMessageLength = cond->maxMessageLength;
    pf->subfulfillment = ffill;

    ffill = calloc(1, sizeof(Fulfillment_t));
    ffill->present = Fulfillment_PR_prefixSha256;
    ffill->choice.prefixSha256 = pf;
    return ffill;
}


static uint32_t prefixSubtypes(CC *cond) {
    return getSubtypes(cond->subcondition) & ~(1 << cc_prefixType.typeId);
}


static CC *prefixFromJSON(cJSON *params, char *err) {
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

    CC *cond = calloc(1, sizeof(CC));
    cond->type = &cc_prefixType;
    cond->maxMessageLength = (unsigned long) mml_item->valuedouble;
    CC *sub = cc_conditionFromJSON(subcond_item, err);
    if (NULL == sub) {
        return NULL;
    }
    cond->subcondition = sub;

    // unsafe
    cond->prefix = base64_decode(prefix_item->valuestring, &cond->prefixLength);
    return cond;
}


static void prefixToJSON(CC *cond, cJSON *params) {
    cJSON_AddNumberToObject(params, "maxMessageLength", (double)cond->maxMessageLength);
    char *b64 = base64_encode(cond->prefix, cond->prefixLength);
    cJSON_AddStringToObject(params, "prefix", b64);
    free(b64);
    cJSON_AddItemToObject(params, "subfulfillment", cc_conditionToJSON(cond->subcondition));
}


int prefixIsFulfilled(CC *cond) {
    return cc_isFulfilled(cond->subcondition);
}


static void prefixFree(CC *cond) {
    free(cond->prefix);
    cc_free(cond->subcondition);
    free(cond);
}


struct CCType cc_prefixType = { 1, "prefix-sha-256", Condition_PR_prefixSha256, 1, &prefixVisitChildren, &prefixFingerprint, &prefixCost, &prefixSubtypes, &prefixFromJSON, &prefixToJSON, &prefixFromFulfillment, &prefixToFulfillment, &prefixIsFulfilled, &prefixFree };
