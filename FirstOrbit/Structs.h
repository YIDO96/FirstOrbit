#pragma once


#define SMALL_NUMBER			(1.e-8f)


struct Pos
{
	float x;
	float y;
};


struct Vector2
{
	float x = 0;
	float y = 0;

	Vector2() {}
	Vector2(float x, float y) : x(x), y(y) {}
	Vector2(POINT pt) : x((float)pt.x), y((float)pt.y) {}

	Vector2 operator+(const Vector2& other) const
	{
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator-(const Vector2& other) const
	{
		return Vector2(x - other.x, y - other.y);
	}

	Vector2 operator-() const
	{
		return Vector2(-x, -y);
	}

	Vector2 operator*(float value) const
	{
		return Vector2(x * value, y * value);
	}

	Vector2 operator/(float value) const
	{
		return Vector2(x / value, y / value);
	}
	void operator+=(const Vector2& other)
	{
		x += other.x;
		y += other.y;
	}

	void operator-=(const Vector2& other)
	{
		x -= other.x;
		y -= other.y;
	}

	void operator*=(float ratio)
	{
		x *= ratio;
		y *= ratio;
	}

	bool operator==(const Vector2& other) const
	{
		return (x == other.x && y == other.y);
	}

	bool operator!=(const Vector2& other) const
	{
		return !(*this == other);
	}

	static Vector2 Lerp(const Vector2& A, const Vector2& B, float ratio)
	{
		return A + (B - A) * ratio;
	}

	// 내적
	// 외적
	// 정규화
	// 크기
	float LengthSquared() const
	{
		return x * x + y * y;
	}
	float Length() const
	{
		return ::sqrt(LengthSquared());
	}

	void Normalize()
	{
		float length = Length();
		if (length < SMALL_NUMBER)
			return;

		x /= length;
		y /= length;
	}

	Vector2 Normalized() const
	{
		Vector2 vec = *this;

		vec.Normalize();

		return vec;
	}

	// Dot 내적
	float Dot(const Vector2& other) const
	{
		return x * other.x + y * other.y;
	}

	// Cross 외적
	float Cross(Vector2 other) const
	{
		return x * other.y - y * other.x;
	}

	Vector2 Rotate(float radian) const
	{
		float cosA = cosf(radian);
		float sinA = sinf(radian);

		return Vector2(x * cosA - y * sinA, x * sinA + y * cosA);
	}
};

inline Vector2 operator*(float value, const Vector2& vec)
{
	return vec * value;
}

inline float GetAngleBetweenVectors(Vector2& v1, Vector2& v2)
{
	float dot = v1.Dot(v2);
	float len1 = v1.Length();
	float len2 = v2.Length();

	float radian = acosf(dot / (len1 * len2));
	float pi_float = std::numbers::pi_v<float>;

	return radian * (180.f / pi_float);
}
inline Vector2 GetRotatedVector(Vector2& v, float degree)
{
	float radian = DegreeToRadian(degree);

	return v.Rotate(radian);
}

// 충돌 판정 결과 (콜라이더 쌍의 정밀 충돌 함수가 채워준다).
// normal은 항상 "판정을 요청한 쪽 -> 상대방" 방향으로 정규화되어 있다.
struct HitResult
{
	Vector2 normal;
	float depth = 0.f;	// 침투 깊이 (얼마나 겹쳤는지)
};

// 열벡터 규약: p' = M * p,  p = (x, y, 1)ᵀ.  m[행][열].
// 
// [ m[0][0]  m[0][1]  m[0][2] ]   [ x ]   [ x' ]
// [ m[1][0]  m[1][1]  m[1][2] ] * [ y ] = [ y' ]
// [ m[2][0]  m[2][1]  m[2][2] ]   [ 1 ]   [ 1  ]
struct Matrix3x3
{
	float m[3][3] =
	{
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};
	


	Matrix3x3() = default;


	// 영행렬
	static Matrix3x3 ZeroMatrix()
	{
		Matrix3x3 mat;
		mat.m[0][0] = 0.0f; mat.m[0][1] = 0.0f; mat.m[0][2] = 0.0f;
		mat.m[1][0] = 0.0f; mat.m[1][1] = 0.0f; mat.m[1][2] = 0.0f;
		mat.m[2][0] = 0.0f; mat.m[2][1] = 0.0f; mat.m[2][2] = 0.0f;

		return mat;
	}

	// 항등 행렬
	static Matrix3x3 Identity()
	{
		Matrix3x3 mat;
		mat.m[0][0] = 1.0f; mat.m[0][1] = 0.0f; mat.m[0][2] = 0.0f;
		mat.m[1][0] = 0.0f; mat.m[1][1] = 1.0f; mat.m[1][2] = 0.0f;
		mat.m[2][0] = 0.0f; mat.m[2][1] = 0.0f; mat.m[2][2] = 1.0f;

		return mat;
	}

	// 이동 행렬
	// [ 1  0  tx]
	// [ 0  1  ty]
	// [ 0  0  1 ]
	static Matrix3x3 Translate(const Vector2& t)
	{
		Matrix3x3 mat = Identity();
		mat.m[0][2] = t.x;
		mat.m[1][2] = t.y;
		return mat;
	}

	// 크기 행렬
	// [ sx  0   0 ]
	// [ 0  sy   0 ]
	// [ 0   0   1 ]
	static Matrix3x3 Scale(const Vector2& s)
	{
		Matrix3x3 mat = Identity();
		mat.m[0][0] = s.x;
		mat.m[1][1] = s.y;
		return mat;
	}
	static Matrix3x3 Scale(const float& s)
	{
		Matrix3x3 mat = Identity();
		mat.m[0][0] = s;
		mat.m[1][1] = s;
		return mat;
	}


	// 회전 행렬
	// [ cos -sin  0 ]
	// [ sin  cos  0 ]
	// [  0    0   1 ]
	static Matrix3x3 Rotate(float rad)
	{
		Matrix3x3 mat = Identity();
		float cosA = ::cosf(rad);
		float sinA = ::sinf(rad);

		mat.m[0][0] = cosA; mat.m[0][1] = -sinA;
		mat.m[1][0] = sinA; mat.m[1][1] = cosA;
		
		return mat;
	}

	static Matrix3x3 Lerp(const Matrix3x3& a, const Matrix3x3& b, float t)
	{
		Matrix3x3 mat;

		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				mat.m[i][j] = a.m[i][j] * (1.f - t) + b.m[i][j] * t;
			}
		}


		return mat;
	}

	// 아핀방식 (2행을 항상 [0 0 1]이라고 가정하고 아예 사용하지 않는다.)
	Matrix3x3 Inverse() const
	{
		// A(좌상단 2x2), t(2열의 0,1행) 구하기
		float A[2][2];
		A[0][0] = m[0][0], A[0][1] = m[0][1], A[1][0] = m[1][0], A[1][1] = m[1][1];

		Vector2 t;
		t.x = m[0][2], t.y = m[1][2];

		// A의 역행렬 구하기
		Matrix3x3 invA;
		float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];

		if (abs(det) < SMALL_NUMBER)
			return ZeroMatrix();

		invA.m[0][0] =  A[1][1] / det;
		invA.m[0][1] = -A[0][1] / det;
		invA.m[1][0] = -A[1][0] / det;
		invA.m[1][1] =  A[0][0] / det;

		// 이동 성분 변환
		Vector2 invAt = -invA.TransformVector(t);

		Matrix3x3 mat;

		mat.m[0][0] = invA.m[0][0], mat.m[0][1] = invA.m[0][1], mat.m[0][2] = invAt.x;
		mat.m[1][0] = invA.m[1][0], mat.m[1][1] = invA.m[1][1], mat.m[1][2] = invAt.y;
		mat.m[2][0] = 0.f, mat.m[2][1] = 0.f, mat.m[2][2] = 1.f;


		return mat;
	}

	// 수반행렬 공식
	Matrix3x3 InverseAdjugate() const
	{
		float det;

		det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
			- m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
			+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

		if (abs(det) < SMALL_NUMBER)
			return ZeroMatrix();

		// 00 01 02
		// 10 11 12
		// 20 21 22
		// 01 <-> 10 / 02 <-> 20 / 12 <-> 21
		Matrix3x3 adj;
		adj.m[0][0] =   m[1][1] * m[2][2] - m[1][2] * m[2][1];
		adj.m[0][1] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]);
		adj.m[0][2] =   m[1][0] * m[2][1] - m[1][1] * m[2][0];

		adj.m[1][0] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]);
		adj.m[1][1] =  (m[0][0] * m[2][2] - m[0][2] * m[2][0]);
		adj.m[1][2] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]);

		adj.m[2][0] =   m[0][1] * m[1][2] - m[0][2] * m[1][1];
		adj.m[2][1] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]);
		adj.m[2][2] =   m[0][0] * m[1][1] - m[0][1] * m[1][0];

		std::swap(adj.m[0][1], adj.m[1][0]);
		std::swap(adj.m[0][2], adj.m[2][0]);
		std::swap(adj.m[1][2], adj.m[2][1]);



		Matrix3x3 inverse;
		inverse = (1 / det) * adj;
		return inverse;
	}

	// 가우스 소거법
	Matrix3x3 InverseGaussJordan() const
	{
		int n = 3;
		Matrix3x3 inv;

		vector<vector<float>> aug(n, vector<float>(2 * n, 0.0f));
		for (int i = 0; i < n; ++i) 
		{
			for (int j = 0; j < n; ++j) 
			{
				aug[i][j] = m[i][j];
			}
			aug[i][i + n] = 1.0; // 오른쪽에 단위행렬 I 배치
		}

		// 가우스 소거법
		for (int i = 0; i < n; ++i)
		{
			// 피벗 선택
			int maxRow = i;
			for (int k = i + 1; k < n; ++k)
			{
				if (abs(aug[k][i]) > abs(aug[maxRow][i]))
				{
					maxRow = k;
				}
			}

			// 행 교환 (큰 값을 위로 올리기)
			std::swap(aug[i], aug[maxRow]);

			// 열 중에 가장 큰 값이  0에 가까우면 역행렬은 존재하지 않음
			if (abs(aug[i][i]) < SMALL_NUMBER)
			{
				// 역행렬이 존재하지 않으면 영행렬로 내보내기;
				return ZeroMatrix();
			}

			// 피벗 행의 주대각의 성분을 1로 정규화 해주기
			float pivot = aug[i][i];
			for (int j = 0; j < 2 * n; ++j)
			{
				aug[i][j] /= pivot;
			}

			// 다른 행렬들의 해당 열 성분을 0으로 만들기
			for (int k = 0; k < n; ++k)
			{
				if (k == i) continue;

				float factor = aug[k][i];
				for (int j = 0; j < 2 * n; ++j)
				{
					aug[k][j] -= factor * aug[i][j];
				}
			}
		}

		// 결과값 복사
		inv = ZeroMatrix();


		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				inv.m[i][j] = aug[i][j + n];
			}
		}

		return inv;
	}


	// operator*
	Matrix3x3 operator*(float scalar) const
	{
		Matrix3x3 result;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				result.m[i][j] = this->m[i][j] * scalar;
			}
		}

		return result;
	}

	friend Matrix3x3 operator*(float scalar, const Matrix3x3& mat)
	{
		return mat * scalar;
	}

	Matrix3x3 operator*(const Matrix3x3& M) const
	{
		Matrix3x3 mat;

		//				i j			j  i	 i  j		 j  i		i j			j i
		//mat.m[0][0] = m[0][0] * M.m[0][0] + m[0][1] * M.m[1][0] + m[0][2] * M.m[2][0];

		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				for (int k = 0; k < 3; ++k)
				{
					mat.m[i][j] += m[i][k] * M.m[k][j];
				}
				//mat.m[i][j] = m[i][0] * M.m[0][j] +
				//			  m[i][1] * M.m[1][j] +
				//			  m[i][2] * M.m[2][j];
			}
		}

		return mat;
	}

	// TransformPoint
	// 점 변환 : (x, y, 1)T 곱셈 -> 이동 영향 받음
	Vector2 TransformPoint(const Vector2& p) const
	{
		float x = m[0][0] * p.x + m[0][1] * p.y + m[0][2] * 1.f;
		float y = m[1][0] * p.x + m[1][1] * p.y + m[1][2] * 1.f;
		return Vector2(x, y);
	}

	// TransformVector
	// 벡터 변환 : (x, y, 0)T 곱셈 -> 이동 무시
	Vector2 TransformVector(const Vector2& v) const
	{
		float x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * 0.f;
		float y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * 0.f;
		return Vector2(x, y);
	}
};