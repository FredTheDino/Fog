namespace Test {

bool math_typedef_s8() { return sizeof(s8) == 1; }
bool math_typedef_u8() { return sizeof(u8) == 1; }
bool math_typedef_s16() { return sizeof(s16) == 2; }
bool math_typedef_u16() { return sizeof(u16) == 2; }
bool math_typedef_s32() { return sizeof(s32) == 4; }
bool math_typedef_u32() { return sizeof(u32) == 4; }
bool math_typedef_f32() { return sizeof(f32) == 4; }
bool math_typedef_s64() { return sizeof(s64) == 8; }
bool math_typedef_u64() { return sizeof(u64) == 8; }
bool math_typedef_f64() { return sizeof(f64) == 8; }

}  // namespace Test
