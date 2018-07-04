#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

int REPCODE_SIZES[] = {
    -1, // Not used
    2,  // DLIS_FSHORT_LEN
    4,  // DLIS_FSINGL_LEN  4
    8,  // DLIS_FSING1_LEN  8
    12, // DLIS_FSING2_LEN  12
    4,  // DLIS_ISINGL_LEN  4
    4,  // DLIS_VSINGL_LEN  4
    8,  // DLIS_FDOUBL_LEN  8
    16, // DLIS_FDOUB1_LEN  16
    24, // DLIS_FDOUB2_LEN  24
    8,  // DLIS_CSINGL_LEN  8
    16, // DLIS_CDOUBL_LEN  16
    1,  // DLIS_SSHORT_LEN  1
    2,  // DLIS_SNORM_LEN   2
    4,  // DLIS_SLONG_LEN   4
    1,  // DLIS_USHORT_LEN  1
    2,  // DLIS_UNORM_LEN   2
    4,  // DLIS_ULONG_LEN   4
    0,  // DLIS_UVARI_LEN   0     //1, 2 or 4
    0,  // DLIS_IDENT_LEN   0    //V
    0,  // DLIS_ASCII_LEN   0    //V
    8,  // DLIS_DTIME_LEN   8
    0,  // DLIS_ORIGIN_LEN  0    //V
    0,  // DLIS_OBNAME_LEN  0    //V
    0,  // DLIS_OBJREF_LEN  0    //V
    0,  // DLIS_ATTREF_LEN  0    //V
    1,  // DLIS_STATUS_LEN  1
    0   // DLIS_UNITS_LEN   0    //V
};

void pack(sized_str_t *lstr, value_t *v) {
	v->u.lstr.len = lstr->len;
	if (MAX_VALUE_SIZE - sizeof(lstr->len) < lstr->len) {
		fprintf(stderr, "sized string is too large for packing");
		exit(-1);
	}
	v->repcode = DLIS_IDENT;
	memcpy(v->u.lstr.buff, lstr->buff, lstr->len);
}
void unpack(value_t *v, sized_str_t *lstr) {
	lstr->len = v->u.lstr.len;
	lstr->buff = v->u.lstr.buff;
}

int parse_ushort(byte_t *data, unsigned int *out) {
	if(data == NULL || out == NULL) {
		fprintf(stderr, "Unexpected NULL");
		return -1;
	}
    uint8_t *pval = (uint8_t *)data;
	*out = *pval;
    return 1;
}
int parse_unorm(byte_t *data, unsigned int *out) {
	if(data == NULL || out == NULL) {
		fprintf(stderr, "Unexpected NULL");
		return -1;
	}
    uint16_t *p_tmp = (uint16_t *)data;
	*out = htons(*p_tmp);
    return 2;
}
int parse_ulong(byte_t * buff, unsigned int *out) {
	if(buff == NULL || out == NULL) {
		fprintf(stderr, "Unexpected NULL");
		return -1;
	}
    unsigned int *p_tmp = (unsigned int *)buff;
	*out = htonl(*p_tmp);
    return 4; 
}


int parse_sshort(byte_t *data, int *out) {
	if(data == NULL || out == NULL) {
		fprintf(stderr, "Unexpected NULL");
		return -1;
	}
    int8_t *pval = (int8_t *)data;
	*out = (int) *pval;
    return 1;
}

int parse_snorm(byte_t *data, int *out){
	if(data == NULL || out == NULL) {
		fprintf(stderr, "Unexpected NULL");
		return -1;
	}
    uint16_t uval = *((uint16_t *)data);
    uval = htons(uval);
    int16_t val = (int16_t)uval;
    *out = (int) val;
	return 2;
}

int parse_slong(byte_t *data, int *out){
	if(data == NULL || out == NULL) {
		fprintf(stderr, "Unexpected NULL");
		return -1;
	}
    uint32_t uval = *((uint32_t*)data);
    uval = htonl(uval);
    int32_t val = (int32_t)uval;
    *out = (int) val;
	return 4;
}

int parse_fsingl(byte_t *data, double *out) {
	if(data == NULL || out == NULL) {
		fprintf(stderr, "Unexpected NULL");
		return -1;
	}
    union fsingl {
        float float_val;
        uint32_t long_val;
    } u;
    uint32_t *tmp = (uint32_t*)data;
    u.long_val = htonl(*tmp);
    *out = u.float_val;
	return 4;
}

int parse_fdoubl(byte_t *data, double *out){
	if(data == NULL || out == NULL) {
		fprintf(stderr, "Unexpected NULL");
		return -1;
	}
    union {
        double double_val;
        uint64_t long_val;
    } u;
    u.double_val = 0.0;

    uint32_t *pdata = (uint32_t *)data;

    uint32_t *first = pdata;
    uint32_t *second = pdata + 1;
    
    uint64_t temp = htonl(*first);

    u.long_val = (temp << 32) + htonl(*second);
    *out = u.double_val;
	return 8;
}

size_t trim(char *out, size_t len, const char *str){
    int length = strlen(str);
    length = (len > length)?length: len;
    if(length == 0) return 0;

    const char *end;
    size_t out_size;

    // Trim leading space
    while(isspace((unsigned char)*str) && length > 0) {
        length--;
        str++;
    }

    if (length == 0) { // All spaces?
        *out = 0;
        return 1;
    }

    if(*str == 0) { // All spaces?
        *out = 0;
        return 1;
    }

    // Trim trailing space
    end = str + length - 1;
    while(end > str && isspace((unsigned char)*end)) {
        end--;
    }
    end++;

    // Set output size to minimum of trimmed string length and buffer size minus 1
    out_size = (end - str);

    // Copy trimmed string and add null terminator
    memcpy(out, str, out_size);
    out[out_size] = 0;

    return out_size;
}

void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

/* check if str is a number (return 1). If it is not, return 0 */
int is_integer(char *str, int len) {
    while (len > 0 && *str != 0 )  {
        if(!isdigit(*str)) {
            return 0;
        }
        str++;
        len--;
    }
    return 1;
}

int parse_ident(byte_t *buff, int buff_len, sized_str_t *output) {
    if (output == NULL || buff == NULL || buff_len <= 1) {
        return -1;
    }
    uint32_t len = 0;
	parse_ushort(buff, &len);
    if ((buff_len-1) < len) return -1; // Not enough data to parse
    output->buff = (buff + 1);
    output->len = len;
    return len + 1;
}

int parse_ascii(byte_t *buff, int buff_len, sized_str_t *output){ 
    if (output == NULL || buff == NULL) {
        return -1;
    }
    uint32_t len = 0;
    int nbytes_read = parse_uvari(buff, buff_len, &len);
    if(nbytes_read <= 0 || buff_len -1 < len) return -1;
    output->buff = (buff + nbytes_read);
    output->len = len;
    return len + nbytes_read;
}

int parse_uvari(byte_t * buff, int buff_len, unsigned int* output){
    int len = 0;
    if(output == NULL || buff == NULL){
        return -1;
    }
    if(!((*buff) & 0x80)) {
        parse_ushort(buff, output);
        len = 1;
    }
    else if(!((*buff) & 0x40)) {
        if(buff_len < 2) return -1;
		uint32_t tmp = 0;
        parse_unorm(buff, &tmp);
		*output = tmp - 0x8000;
        len = 2;
    }
    else {
        if(buff_len < 4) return -1;
		uint32_t tmp = 0;
		parse_ulong(buff, &tmp); 
        *output = tmp - 0xC0000000;
        len = 4;
    }
    return len;
}

int parse_value(byte_t* buff, int buff_len, int repcode, value_t *output){
    sized_str_t str;
    int repcode_len = 0;

    if (repcode >= DLIS_REPCODE_MAX) {
        fprintf(stderr, "encounter wrong repcode\n");
        exit(-1);
    }
    repcode_len = REPCODE_SIZES[repcode];
	
	output->repcode = repcode;
    if (buff_len < repcode_len) return -1;
    switch(repcode) {
        case DLIS_FSHORT:
            break;
        case DLIS_FSINGL:
            parse_fsingl(buff, &output->u.double_val);
            break;
        case DLIS_FSING1:
            break;
        case DLIS_FSING2:
            break;
        case DLIS_ISINGL:
            break;
        case DLIS_VSINGL:
            break;
        case DLIS_FDOUBL:
            parse_fdoubl(buff, &output->u.double_val);
            break;
        case DLIS_FDOUB1:
            break;
        case DLIS_FDOUB2:
            break;
        case DLIS_CSINGL:
            break;
        case DLIS_CDOUBL:
            break;
        case DLIS_SSHORT:
            parse_sshort(buff, &output->u.int_val);
            break;
        case DLIS_SNORM:
            parse_snorm(buff, &output->u.int_val);
            break;
        case DLIS_SLONG:
            parse_slong(buff, &output->u.int_val);
            break;
        case DLIS_USHORT:
            parse_ushort(buff, &output->u.uint_val);
            break;
        case DLIS_UNORM:
            parse_unorm(buff, &output->u.uint_val);
            break;
        case DLIS_ULONG:
            parse_ulong(buff, &output->u.uint_val);
            break;
        case DLIS_UVARI:
        case DLIS_ORIGIN:
            repcode_len = parse_uvari(buff, buff_len, &output->u.uint_val);
            break;
        case DLIS_IDENT:
        case DLIS_UNITS:
            repcode_len = parse_ident(buff, buff_len, &str);
			pack(&str, output);
            break;
        case DLIS_ASCII:
            repcode_len = parse_ascii(buff, buff_len, &str);
			pack(&str, output);
            break;
        case DLIS_OBNAME:
        case DLIS_OBJREF:
        case DLIS_ATTREF:
            break;
        case DLIS_DTIME:
            break;
        case DLIS_STATUS:
            break;
    }
    return repcode_len;
}

int parse_values(byte_t *buff, int buff_len, int val_cnt, int repcode) {
	printf("parse_values: %d %d %d", buff_len, val_cnt, repcode);
	if(buff == NULL || buff_len <= 0 || val_cnt <= 0) {
		return -1;
	}
	byte_t obuff[1024];
	value_t out;

	int len = parse_value(buff, buff_len, repcode, &out);
	if (len <= 0) return -1;
	
    return len;
}

int parse_obname(byte_t *buff, int buff_len, obname_t* obname){
    if(buff == NULL || obname == NULL || buff_len <= 0)  return -1;

    int origin_len = parse_uvari(buff, buff_len, &obname->origin);
    if(origin_len <= 0) return -1;

    parse_ushort(buff + origin_len, &obname->copy_number);

    int name_len = parse_ident(buff + origin_len + 1, buff_len - origin_len - 1, &obname->name);
    if(name_len <= 0) return -1;
    return origin_len + 1 + name_len;
}

void print_str(sized_str_t str){
    printf("%.*s\n", str.len, str.buff);
}
void print_obname(obname_t *obname){
    printf("\torigin: %d, copy_number: %d, name: %.*s\n", 
        obname->origin, obname->copy_number, obname->name.len, obname->name.buff);
}
