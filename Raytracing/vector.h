#pragma once

#include <iostream>

struct vector {
	float x = 0;
	float y = 0;

	static vector zero() {
		return { 0, 0 };
	}

	float length() {
		return sqrt(x * x + y * y);
	}

	vector round() {
		return vector{ roundf(x), roundf(y) };
	}

	// Addition
	vector operator+ (const vector& other) const {
		return { x + other.x, y + other.y };
	}

	vector operator+ (float b) const {
		return { x + b, y + b };
	}

	vector operator+= (const vector& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	vector operator+= (float other) {
		x += other;
		y += other;
		return *this;
	}

	// Subtraction
	vector operator-(const vector& other) const {
		return { x - other.x, y - other.y };
	}

	vector operator- (float b) const {
		return { x - b, y - b };
	}

	vector operator-= (const vector& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	vector operator-= (float other) {
		x -= other;
		y -= other;
		return *this;
	}

	// Unary minus
	vector operator-() const {
		return { -x, -y };
	}

	// Multiplication
	vector operator* (vector b) const {
		return { x * b.x, y * b.y };
	}

	vector operator*(float scalar) const {
		return { x * scalar, y * scalar };
	}

	vector operator*= (vector other) {
		x *= other.x;
		y *= other.y;
		return *this;
	}

	vector operator*= (float other) {
		x *= other;
		y *= other;
		return *this;
	}

	// Division
	vector operator/ (vector b) const {
		return { x / b.x, y / b.y };
	}

	vector operator/ (float scalar) const {
		return { x / scalar, y / scalar };
	}

	vector operator/= (vector other) {
		x /= other.x;
		y /= other.y;
		return *this;
	}

	vector operator/= (float other) {
		x /= other;
		y /= other;
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& os, const vector& v) {
		os << "(" << v.x << ", " << v.y << ")";
		return os;
	}
};

struct Rect {
	float x = 0;
	float y = 0;
	float w = 0;
	float h = 0;

	friend std::ostream& operator<<(std::ostream& os, const Rect& r) {
		os << "(" << r.x << ", " << r.y << ", " << r.w << ", " << r.h << ")";
		return os;
	}
};