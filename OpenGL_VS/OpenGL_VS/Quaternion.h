/* 
David Tu
david.tu2@csu.fullerton.edu
 
A quaternion implementation.
It needs a bit more work but it has the neccessary functions for this project
*/

class Quaternion{
public:
  //Required: _angle should always be in radians
  Quaternion(): _name("No name"){ }

  Quaternion(glm::vec3 axis, float angle, std::string name): _name(name){
	_axis.x = axis.x * sin(angle/2);
	_axis.y = axis.y * sin(angle/2);
	_axis.z = axis.z * sin(angle/2);
	_angle = cos(angle/2);
  }

  //Copy constructor
  Quaternion(glm::vec3 axis, float angle){
	_axis.x = axis.x;
	_axis.y = axis.y;
	_axis.z = axis.z;
	_angle = angle;
	_name = "Copy";
  }

  ~Quaternion(){}

  float x(){
	return _axis.x;
  }

  float y(){
	return _axis.y;
  }

  float z(){
	return _axis.z;
  }

  glm::vec3 axis(){
	return _axis;
  }

  float w(){
	return _angle;
  }

  std::string name(){
	return _name;
  }

  void setName(std::string name){
	_name = name;
  }

  float length(){
	//Should always be 1
	return sqrt(_axis.x*_axis.x + _axis.y*_axis.y + _axis.z*_axis.z + _angle*_angle);
  }

  Quaternion normalize(){
	glm::vec3 n;
	float w;
	float l = length();

	w = _angle /= l;
	n = _axis /= l;

	Quaternion normalized(n, w);
	normalized.setName("Normal of " + _name);
	return normalized;
  }

  //A.k.a. Invert
  Quaternion conjugate(){
	glm::vec3 c;
	float w;

	//w stays the same
	w = _angle;
	c = -_axis;

	Quaternion conjugated(c, w);
	conjugated.setName("Invert of " + _name);
	return conjugated;
  }

  /*This is c = b * a, remember that order matters. 
  Similarily to transformations, the 1st operand, a, starts at the right! 
  Multiplying quaternions together combines the rotations*/
  Quaternion multiply(Quaternion& b){
	glm::vec3 c;
	float w;
	//Making a copy of this quaternion so that the math would be easier to understand
	Quaternion a(_axis, _angle);
	a.setName(_name);

	w = a.w()*b.w() + glm::dot(_axis, b.axis());
	c = a.axis()*b.w() + b.axis()*a.w() + glm::cross(b.axis(), a.axis());

	/*//Same as:
	c.x = a.w()*b.x() + a.x()*b.w() + a.y()*b.z() - a.z()*b.y();
	c.y = a.w()*b.y() - a.x()*b.z() + a.y()*b.w() + a.z()*b.x();
	c.z = a.w()*b.z() + a.x()*b.y() - a.y()*b.x() + a.z()*b.w();
	w = a.w()*b.w() - a.x()*b.x() - a.y()*b.y() - a.z()*b.z();*/

	Quaternion product(c, w);
	product.setName(b.name() + " * " + _name);

	if(product.length() > 1){
		throw new std::invalid_argument("Length is greater than 1.");
	}

	return product;
  }

  glm::mat4 getRotationMatrix(){
	float x2 = _axis.x*_axis.x;
	float y2 = _axis.y*_axis.y;
	float z2 = _axis.z*_axis.z;
	float xy = _axis.x*_axis.y;
	float xz = _axis.x*_axis.z;
	float yz = _axis.y*_axis.z;
	float wx = _angle*_axis.x;
	float wy = _angle*_axis.y;
	float wz = _angle*_axis.z;

	//Added in column major order (OpenGL's style)
	glm::mat4 rotation(	1.0f - 2.0f*y2 - 2.0f*z2,
						2.0f*xy - 2.0f*wz,
						2.0f*xz + 2.0f*wy,
						0.0f,

						2.0f*xy + 2.0f*wz,
						1.0f - 2.0f*x2 -2.0f*z2,
						2.0f*yz - 2.0f*wx,
						0.0f,

						2.0f*xz - 2.0f*wy,
						2.0f*yz + 2.0f*wx,
						1.0f - 2.0f*x2 -2.0f*y2,
						0.0f,

						0.0f, 0.0f, 0.0f, 1.0f);
	return rotation;
  }

  void debug(){
	std::cerr << "Name: " << _name << std::endl;
	std::cerr << "x: " << _axis.x << std::endl;
	std::cerr << "y: " << _axis.y << std::endl;
	std::cerr << "z: " << _axis.z << std::endl;
	std::cerr << "Angle: " << _angle << std::endl << std::endl;
  }

private:
  glm::vec3 _axis;
  float _angle;
  std::string _name;
};
