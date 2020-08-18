DROP TYPE IF EXISTS textenc CASCADE;
DROP TABLE IF EXISTS textenc_test;
DROP TABLE IF EXISTS text_test;

-- Typdefinition textenc

CREATE FUNCTION textenc_in(cstring)
    RETURNS textenc
    AS '/var/lib/postgresql/textenc.so'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION textenc_out(textenc)
    RETURNS cstring
    AS '/var/lib/postgresql/textenc.so'
    LANGUAGE C IMMUTABLE STRICT;


CREATE TYPE textenc (
   internallength = VARIABLE,
   input = textenc_in,
   output = textenc_out,
   alignment = int4
);

-- Strategien

CREATE FUNCTION textenc_lt(textenc, textenc) RETURNS bool
    AS '/var/lib/postgresql/textenc.so', 'textenc_lt'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR < (
   leftarg = textenc, rightarg = textenc, procedure = textenc_lt,
   commutator = > , negator = >= ,
   restrict = scalarltsel, join = scalarltjoinsel
);

CREATE FUNCTION textenc_lt_eq(textenc, textenc) RETURNS bool
    AS '/var/lib/postgresql/textenc.so', 'textenc_lt_eq'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR <= (
   leftarg = textenc, rightarg = textenc, procedure = textenc_lt_eq,
   commutator = >= , negator = > ,
   restrict = scalarlesel, join = scalarlejoinsel
);

CREATE FUNCTION textenc_eq(textenc, textenc) RETURNS bool
    AS '/var/lib/postgresql/textenc.so', 'textenc_eq'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR = (
   leftarg = textenc, rightarg = textenc, procedure = textenc_eq,
   commutator = =,
   restrict = eqsel, join = eqjoinsel
);

CREATE FUNCTION textenc_gt_eq(textenc, textenc) RETURNS bool
    AS '/var/lib/postgresql/textenc.so', 'textenc_gt_eq'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR >= (
   leftarg = textenc, rightarg = textenc, procedure = textenc_gt_eq,
   commutator = <=, neagtor = < ,
   restrict = scalargesel, join = scalargejoinsel
);

CREATE FUNCTION textenc_gt(textenc, textenc) RETURNS bool
    AS '/var/lib/postgresql/textenc.so', 'textenc_gt'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR > (
   leftarg = textenc, rightarg = textenc, procedure = textenc_gt,
   commutator = <, neagtor = <= ,
   restrict = scalargtsel, join = scalargtjoinsel
);

-- Support Funktion

CREATE FUNCTION textenc_compare(textenc, textenc) RETURNS int4
   AS '/var/lib/postgresql/textenc.so', 'textenc_compare' LANGUAGE C IMMUTABLE STRICT;

CREATE OPERATOR CLASS textenc_ops
    DEFAULT FOR TYPE textenc USING btree AS
        OPERATOR        1       < ,
        OPERATOR        2       <= ,
        OPERATOR        3       = ,
        OPERATOR        4       >= ,
        OPERATOR        5       > ,
        FUNCTION        1       textenc_compare(textenc, textenc);
