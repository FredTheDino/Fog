#pragma once

struct Mat4
{
	union 
	{
		// Row x Collumn
		struct
		{
			real _00, _01, _02, _03;
			real _10, _11, _12, _13;
			real _20, _21, _22, _23;
			real _30, _31, _32, _33;
		};
		real _[4][4];
		real __[16];
	};

	Mat4 operator* (real s)
	{
		Mat4 out;
		for (u8 i = 0; i < 16; i++)
		{
			out.__[i] = __[i] * s;
		}
		return out;
	}

	void operator*= (real s)
	{
		for (u8 i = 0; i < 16; i++)
		{
			__[i] = __[i] * s;
		}
	}

	Mat4 operator* (Mat4 m)
	{
		Mat4 out = {};
		for (u8 row = 0; row < 4; row++)
		{
			for (u8 col = 0; col < 4; col++)
			{
				for (u8 i = 0; i < 4; i++)
				{
					out._[row][col] += _[row][i] * m._[i][col];
				}
			}
		}
		return out;
	}
	
	void operator*= (Mat4 m)
	{
		*this = (*this) * m;
	}

	Vec4 operator* (Vec4 v)
	{
		return
		{
			(real) v.x * _00 + v.y * _01 + v.z * _02 + v.w * _03,
			(real) v.x * _10 + v.y * _11 + v.z * _12 + v.w * _13,
			(real) v.x * _20 + v.y * _21 + v.z * _22 + v.w * _23,
			(real) v.x * _30 + v.y * _31 + v.z * _32 + v.w * _33
		};
	}

	Vec3 operator* (Vec3 v)
	{
		return V3((*this) * V4(v));
	}

	bool operator== (Mat4 m)
	{
		for (u8 i = 0; i < 16; i++)
		{
			if (__[i] != m.__[i])
				return false;
		}
		return true;
	}

	Mat4 operator+ (Mat4 m)
	{
		Mat4 out;
		for (u8 i = 0; i < 16; i++)
		{
			out.__[i] = m.__[i] + __[i];
		}
		return out;
	}

	void operator+= (Mat4 m)
	{
		for (u8 i = 0; i < 16; i++)
		{
			__[i] = m.__[i] + __[i];
		}
	}
};

real *data_ptr(Mat4 &m)
{
	return &m.__[0];
}

Mat4 create_identity()
{
	return
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 
		0.0f, 0.0f, 0.0f, 1.0f
	};
}

Mat4 translate(Mat4 m, Vec3 v)
{
	m._03 += v.x;
	m._13 += v.y;
	m._23 += v.z;

	return m;
}

Mat4 scale(Mat4 m, Vec3 v)
{
	m._00 *= v.x;
	m._10 *= v.x;
	m._20 *= v.x;
	m._01 *= v.y;
	m._11 *= v.y;
	m._21 *= v.y;
	m._02 *= v.z;
	m._12 *= v.z;
	m._22 *= v.z;
	return m;
}

Mat4 scale(Mat4 m, real scalar)
{
	m._00 *= scalar;
	m._10 *= scalar;
	m._20 *= scalar;
	m._01 *= scalar;
	m._11 *= scalar;
	m._21 *= scalar;
	m._02 *= scalar;
	m._12 *= scalar;
	m._22 *= scalar;

	return m;
}

Mat4 M4(Q q)
{
    // TODO(ed): We can do some of these calculations only once.
	return
	{
		1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z, 
		2.0f * q.x * q.y - 2.0f * q.w * q.z, 
		2.0f * q.w * q.y + 2.0f * q.x * q.z,
		0.0f,

		2.0f * q.x * q.y + 2.0f * q.w * q.z, 
		1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z, 
		2.0f * q.y * q.z - 2.0f * q.w * q.x,
		0.0f,

		2.0f * q.x * q.z - 2.0f * q.w * q.y, 
		2.0f * q.w * q.x + 2.0f * q.y * q.z,
		1.0f - 2.0f * q.x * q.x - 2 * q.y * q.y, 
		0.0f,

		0.0f,
		0.0f,
		0.0f,
		1.0f,
	};
};

Mat4 rotate(Mat4 m, Quaternion q)
{
	return m * M4(q);
}

Mat4 create_skew_symmetric(Vec3 v)
{
	Mat4 m;
	m._00 = m._11 = m._22 = m._33 = 0.0f;
	m._10 =  v.z;
	m._01 = -v.z;
	m._02 =  v.y;
	m._20 = -v.y;
	m._13 =  v.z;
	m._31 = -v.z;
	return m;
}

Mat4 zero_transform(Mat4 m)
{
	m._03 = 0.0f;
	m._13 = 0.0f;
	m._23 = 0.0f;
	m._33 = 1.0f;
	return m;
}

// NOTE: "near", and "far" as variable names cause syntax errors on Windows.
Mat4 create_perspective_projection(real fov, real aspect_ratio, real near_clip, real far_clip)
{
	real w = tan(fov / 2.0f);
	
	Mat4 m = {};
	m._00 = 1.0f / (aspect_ratio * w);
	m._11 = 1.0f / w;
	m._22 = - (far_clip + near_clip) / (far_clip - near_clip);
	m._32 = - 1.0f;
	m._23 = - (2.0f * far_clip * near_clip) / (far_clip - near_clip);
	return m;
}

Mat4 transpose(Mat4 m)
{
	Mat4 t = {};

	for (u8 i = 0; i < 4; i++)
		for (u8 j = 0; j < 4; j++)
			t._[i][j] = m._[j][i];

	return t;
}

//TODO: Rewrite function to avoid usage of 'transpose'
Mat4 inverse(Mat4 m)
{
	Mat4 inv = {};
	
	m = transpose(m);
	
	inv.__[0] = m.__[5]  * m.__[10] * m.__[15] - 
		m.__[5]  * m.__[11] * m.__[14] - 
		m.__[9]  * m.__[6]  * m.__[15] + 
		m.__[9]  * m.__[7]  * m.__[14] +
		m.__[13] * m.__[6]  * m.__[11] - 
		m.__[13] * m.__[7]  * m.__[10];

    inv.__[4] = -m.__[4]  * m.__[10] * m.__[15] + 
		m.__[4]  * m.__[11] * m.__[14] + 
		m.__[8]  * m.__[6]  * m.__[15] - 
		m.__[8]  * m.__[7]  * m.__[14] - 
		m.__[12] * m.__[6]  * m.__[11] + 
		m.__[12] * m.__[7]  * m.__[10];

    inv.__[8] = m.__[4]  * m.__[9] * m.__[15] - 
		m.__[4]  * m.__[11] * m.__[13] - 
		m.__[8]  * m.__[5] * m.__[15] + 
		m.__[8]  * m.__[7] * m.__[13] + 
		m.__[12] * m.__[5] * m.__[11] - 
		m.__[12] * m.__[7] * m.__[9];

    inv.__[12] = -m.__[4]  * m.__[9] * m.__[14] + 
		m.__[4]  * m.__[10] * m.__[13] +
		m.__[8]  * m.__[5] * m.__[14] - 
		m.__[8]  * m.__[6] * m.__[13] - 
		m.__[12] * m.__[5] * m.__[10] + 
		m.__[12] * m.__[6] * m.__[9];

    inv.__[1] = -m.__[1]  * m.__[10] * m.__[15] + 
		m.__[1]  * m.__[11] * m.__[14] + 
		m.__[9]  * m.__[2] * m.__[15] - 
		m.__[9]  * m.__[3] * m.__[14] - 
		m.__[13] * m.__[2] * m.__[11] + 
		m.__[13] * m.__[3] * m.__[10];

    inv.__[5] = m.__[0]  * m.__[10] * m.__[15] - 
		m.__[0]  * m.__[11] * m.__[14] - 
		m.__[8]  * m.__[2] * m.__[15] + 
		m.__[8]  * m.__[3] * m.__[14] + 
		m.__[12] * m.__[2] * m.__[11] - 
		m.__[12] * m.__[3] * m.__[10];

    inv.__[9] = -m.__[0]  * m.__[9] * m.__[15] + 
		m.__[0]  * m.__[11] * m.__[13] + 
		m.__[8]  * m.__[1] * m.__[15] - 
		m.__[8]  * m.__[3] * m.__[13] - 
		m.__[12] * m.__[1] * m.__[11] + 
		m.__[12] * m.__[3] * m.__[9];

    inv.__[13] = m.__[0]  * m.__[9] * m.__[14] - 
		m.__[0]  * m.__[10] * m.__[13] - 
		m.__[8]  * m.__[1] * m.__[14] + 
		m.__[8]  * m.__[2] * m.__[13] + 
		m.__[12] * m.__[1] * m.__[10] - 
		m.__[12] * m.__[2] * m.__[9];

    inv.__[2] = m.__[1]  * m.__[6] * m.__[15] - 
		m.__[1]  * m.__[7] * m.__[14] - 
		m.__[5]  * m.__[2] * m.__[15] + 
		m.__[5]  * m.__[3] * m.__[14] + 
		m.__[13] * m.__[2] * m.__[7] - 
		m.__[13] * m.__[3] * m.__[6];

    inv.__[6] = -m.__[0]  * m.__[6] * m.__[15] + 
		m.__[0]  * m.__[7] * m.__[14] + 
		m.__[4]  * m.__[2] * m.__[15] - 
		m.__[4]  * m.__[3] * m.__[14] - 
		m.__[12] * m.__[2] * m.__[7] + 
		m.__[12] * m.__[3] * m.__[6];

    inv.__[10] = m.__[0]  * m.__[5] * m.__[15] - 
		m.__[0]  * m.__[7] * m.__[13] - 
		m.__[4]  * m.__[1] * m.__[15] + 
		m.__[4]  * m.__[3] * m.__[13] + 
		m.__[12] * m.__[1] * m.__[7] - 
		m.__[12] * m.__[3] * m.__[5];

    inv.__[14] = -m.__[0]  * m.__[5] * m.__[14] + 
		m.__[0]  * m.__[6] * m.__[13] + 
		m.__[4]  * m.__[1] * m.__[14] - 
		m.__[4]  * m.__[2] * m.__[13] - 
		m.__[12] * m.__[1] * m.__[6] + 
		m.__[12] * m.__[2] * m.__[5];

    inv.__[3] = -m.__[1] * m.__[6] * m.__[11] + 
		m.__[1] * m.__[7] * m.__[10] + 
		m.__[5] * m.__[2] * m.__[11] - 
		m.__[5] * m.__[3] * m.__[10] - 
		m.__[9] * m.__[2] * m.__[7] + 
		m.__[9] * m.__[3] * m.__[6];

    inv.__[7] = m.__[0] * m.__[6] * m.__[11] - 
		m.__[0] * m.__[7] * m.__[10] - 
		m.__[4] * m.__[2] * m.__[11] + 
		m.__[4] * m.__[3] * m.__[10] + 
		m.__[8] * m.__[2] * m.__[7] - 
		m.__[8] * m.__[3] * m.__[6];

    inv.__[11] = -m.__[0] * m.__[5] * m.__[11] + 
		m.__[0] * m.__[7] * m.__[9] + 
		m.__[4] * m.__[1] * m.__[11] - 
		m.__[4] * m.__[3] * m.__[9] - 
		m.__[8] * m.__[1] * m.__[7] + 
		m.__[8] * m.__[3] * m.__[5];

    inv.__[15] = m.__[0] * m.__[5] * m.__[10] - 
		m.__[0] * m.__[6] * m.__[9] - 
		m.__[4] * m.__[1] * m.__[10] + 
		m.__[4] * m.__[2] * m.__[9] + 
		m.__[8] * m.__[1] * m.__[6] - 
		m.__[8] * m.__[2] * m.__[5];
	
	real det = m.__[0] * inv.__[0] + m.__[1] * inv.__[4] + m.__[2] * inv.__[8] + m.__ [3] * inv.__[12];

	if (det == 0)
		return {};

	det = 1.0f / det;

	for (u8 i = 0; i < 16; i++)
		inv.__[i] *= det;

	return transpose(inv);
}
