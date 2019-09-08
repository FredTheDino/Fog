#pragma once

struct Q;
Q normalize(Q q);

struct Q
{
	union 
	{
		struct
		{
			real x, y, z, w;
		};
		struct
		{
			Vec3 v;
			// real w; This W should be in the same place no matter what...
		};
		real _[4];
	};


	Q operator- ()
	{
		return {-x, -y, -z, w};
	}

	Q operator+ (Q q)
	{
		return {x + q.x, y + q.y, z + q.z, w + q.w};
	}

	Q operator- (Q q)
	{
		return {x - q.x, y - q.y, z - q.z, w - q.w};
	}

	void operator+= (Q q)
	{
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;
	}

	void operator-= (Q q)
	{
		x -= q.x;
		y -= q.y;
		z -= q.z;
		w -= q.w;
	}

	Q operator* (real s)
	{
		return 
		{
			x * s,
			y * s,
			z * s,
			w * s
		};
	}
	
	Q operator/ (real s)
	{
		real d = 1.0f / s;
		return (*this) * d;
	}

	Q operator* (Q o)
	{
		Q result;
		result.v = cross(v, o.v) + o.v * w + v * o.w;
		result.w = w * o.w - dot(v, o.v);
		/*
		return
		{
			w * o.x + x * o.w + y * o.z - z * o.y,
			w * o.y - x * o.z + y * o.w + z * o.x,
			w * o.z + x * o.y - y * o.x + z * o.w,
			w * o.w - x * o.x - y * o.y + z * o.z,
		};
		*/
		return result;
	}

	void operator*= (Q o)
	{
		*this = (*this) * (o);
	}

	Q operator/ (Q o)
	{
		return (*this) * (-o);
	}

	void operator/= (Q o)
	{
		*this = (*this) / o;
	}

	Vec3 operator* (Vec3 u)
	{
		Q p = {u.x, u.y, u.z, 0.0f};
		Q q = (*this);
		Q tmp = q * p;
		tmp = tmp * -q;
		return {tmp.x, tmp.y, tmp.z};
	}

	Vec3 operator/ (Vec3 u)
	{
		Q p = {u.x, u.y, u.z, 0.0f};
		Q q = -(*this);
		Q tmp = q * p;
		tmp = tmp * -q;
		return {tmp.x, tmp.y, tmp.z};
	}
};

typedef Q Quat;
typedef Q Quaternion;

real length_squared(Q q)
{
	return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
}

real length(Q q)
{
	return sqrt(length_squared(q));
}

Q normalize(Q q)
{
	return q / length(q);
}

Q conjugate(Q q)
{
	return {-q.x, -q.y, -q.z, q.w};
}

Q lerp(Q q1, real lerp, Q q2)
{
	return q1 * lerp + (q2 * (1.0f - lerp));
}

Q toQ(real roll, real pitch, real yaw)
{
	real cy = cos(yaw   * 0.5);
	real sy = sin(yaw   * 0.5);
	real cr = cos(roll  * 0.5);
	real sr = sin(roll  * 0.5);
	real cp = cos(pitch * 0.5);
	real sp = sin(pitch * 0.5);
	
	return
	{
		cy * sr * cp - sy * cr * sp,
		cy * cr * sp + sy * sr * cp,
		sy * cr * cp - cy * sr * sp,
		cy * cr * cp + sy * sr * sp
	};
}

Q toQ(Vec3 axis, real angle)
{
	real theta = angle / 2.0f;
	Q result;
	result.v = axis * sin(theta);
	result.w = (real) cos(theta);
	result = normalize(result);
	return result;
}

Vec3 to_euler(Q q)
{
	// Roll
	real roll = atan2(2.0f * (q.w * q.x + q.y * q.z),
                      1.0f - 2.0f * (q.x * q.x + q.y * q.y));

	// Pitch
	real sinp = 2.0f * (q.w * q.y - q.z * q.x);
	real pitch;
	if (fabs(sinp) >= 1.0f)
		pitch = copysign(PI / 2.0f, sinp);
	else
		pitch = asin(sinp);
	
	// Yaw
	real yaw = atan2(2.0f * (q.w * q.z + q.x * q.y),
                     1.0f - 2.0f * (q.y * q.y + q.z * q.z));
	
	return { roll, pitch, yaw };
}

