//camera.cpp
#include "camera.h" 
using namespace std;

Camera::Camera() {
	//initialize camera properties
	camera_mode = FREE; //initialize camera to free mode
	camera_up = glm::vec3(0, 1, 0);//initialize the up direction to up the y-axis
	field_of_view = 45; //set field of view to 45
	camera_position_delta = glm::vec3(0, 0, 0); //change in camera position is 0
	camera_scale = .5f;
	max_pitch_rate = 5;
	max_heading_rate = 5;
	move_camera = false; //set camera stationary
}
Camera::~Camera() {
}

void Camera::Reset() {
	//reset camera
	camera_up = glm::vec3(0, 1, 0); //set up direction to up the y-axis
}

void Camera::Update() {
	camera_direction = glm::normalize(camera_look_at - camera_position);
	//need to set the matrix state. this is only important because lighting doesn't work if this isn't done
	glViewport(viewport_x, viewport_y, window_width, window_height);

	if (camera_mode == ORTHO) {
		//our projection matrix will be an orthogonal one in this case
		//if the values are not floating point, this command does not work properly
		//need to multiply by aspect!!! (otherise will not scale properly)
		projection = glm::ortho(-1.5f * float(aspect), 1.5f * float(aspect), -1.5f, 1.5f, -10.0f, 10.f);
	} else if (camera_mode == FREE) {
		projection = glm::perspective(field_of_view, aspect, near_clip, far_clip);
		//detmine axis for pitch rotation
		glm::vec3 axis = glm::cross(camera_direction, camera_up);
		//compute quaternion for pitch based on the camera pitch angle
		glm::quat pitch_quat = glm::angleAxis(camera_pitch, axis);
		//determine heading quaternion from the camera up vector and the heading angle
		glm::quat heading_quat = glm::angleAxis(camera_heading, camera_up);
		//add the two quaternions
		glm::quat temp = glm::cross(pitch_quat, heading_quat);
		temp = glm::normalize(temp);
		//update the direction from the quaternion
		camera_direction = glm::rotate(temp, camera_direction);
		//add the camera delta
		camera_position += camera_position_delta;
		//set the look at to be infront of the camera
		camera_look_at = camera_position + camera_direction * 1.0f;
		//damping for smooth camera
		camera_heading *= .5;
		camera_pitch *= .5;
		camera_position_delta = camera_position_delta * .8f;
	}
	//compute the MVP
	view = glm::lookAt(camera_position, camera_look_at, camera_up); // set the view to the glutfunction lookAt
	model = glm::mat4(1.0f);
	MVP = projection * view * model;
}

//Setting Functions
void Camera::SetMode(CameraType cam_mode) {
	//set camera mode
	camera_mode = cam_mode;
	camera_up = glm::vec3(0, 1, 0);  //set up direction to up the y-axis
}

void Camera::SetPosition(glm::vec3 pos) {
	//set camera position
	camera_position = pos;
}

void Camera::SetLookAt(glm::vec3 pos) {
	//set what camera looks at
	camera_look_at = pos;
}
void Camera::SetFOV(double fov) {
	//set field of view
	field_of_view = fov;
}
void Camera::SetViewport(int loc_x, int loc_y, int width, int height) {
	//set viewport
	viewport_x = loc_x;
	viewport_y = loc_y;
	window_width = width;
	window_height = height;
	//need to use doubles division here, it will not work otherwise and it is possible to get a zero aspect ratio with integer rounding
	aspect = double(width) / double(height);
	;
}
void Camera::SetClipping(double near_clip_distance, double far_clip_distance) {
	near_clip = near_clip_distance;
	far_clip = far_clip_distance;
}

//Move Function
void Camera::Move(CameraDirection dir) { //move based on parameter
	if (camera_mode == FREE) { // if camera can move
		switch (dir) { //go to case of the parameter
			case UP: //if dir is up start here
				camera_position_delta += camera_up * camera_scale; //move camera up
				break; //exit switch
			case DOWN: //if dir is down start here
				camera_position_delta -= camera_up * camera_scale; //move camera down
				break;
			case LEFT: //if dir is left start here
				camera_position_delta -= glm::cross(camera_direction, camera_up) * camera_scale; //move camera left
				break; //exit switch
			case RIGHT: //if dir is right start here
				camera_position_delta += glm::cross(camera_direction, camera_up) * camera_scale; //move camera right
				break; //exit switch
			case FORWARD: //if dir is forward start here
				camera_position_delta += camera_direction * camera_scale; //move camera forward
				break; //exit switch
			case BACK: //if dir is back start here
				camera_position_delta -= camera_direction * camera_scale; //move camera back
				break; //exit switch
		}
	}
}
void Camera::ChangePitch(float degrees) {
	//Check bounds with the max pitch rate so that we aren't moving too fast
	if (degrees < -max_pitch_rate) { //degrees moving too fast in negative direction
		degrees = -max_pitch_rate; //change degrees to max value in negative direction
	} else if (degrees > max_pitch_rate) { //degrees moving too fast in positive direction
		degrees = max_pitch_rate; //change degrees to max value in positive direction
	}
	camera_pitch += degrees; //change camera pitch

	//Check bounds for the camera pitch
	if (camera_pitch > 360.0f) { // camera pitch goes over 360 degrees in one direction
		camera_pitch -= 360.0f; //rotate camera 360 degrees to normalize camera_pitch value
	} else if (camera_pitch < -360.0f) { // camera pitch goes over 360 degrees in one direction
		camera_pitch += 360.0f; //rotate camera 360 degrees to normalize camera_pitch value
	}
}
void Camera::ChangeHeading(float degrees) {
	//Check bounds with the max heading rate so that we aren't moving too fast
	if (degrees < -max_heading_rate) { //degrees moving too fast in negative direction
		degrees = -max_heading_rate; //change degrees to max value in negative direction
	} else if (degrees > max_heading_rate) { //degrees moving too fast in positive direction
		degrees = max_heading_rate; //change degrees to max value in positive direction
	}
	//This controls how the heading is changed if the camera is pointed straight up or down
	//The heading delta direction changes
	if (camera_pitch > 90 && camera_pitch < 270 || (camera_pitch < -90 && camera_pitch > -270)) {
		camera_heading -= degrees;
	} else {
		camera_heading += degrees;
	}
	//Check bounds for the camera heading
	if (camera_heading > 360.0f) {
		camera_heading -= 360.0f;
	} else if (camera_heading < -360.0f) {
		camera_heading += 360.0f;
	}
}
void Camera::Move2D(int x, int y) {
	//compute the mouse delta from the previous mouse position
	glm::vec3 mouse_delta = mouse_position - glm::vec3(x, y, 0);
	//if the camera is moving, meaning that the mouse was clicked and dragged, change the pitch and heading
	if (move_camera) {
		ChangeHeading(.08f * mouse_delta.x);
		ChangePitch(.08f * mouse_delta.y);
	}
	mouse_position = glm::vec3(x, y, 0);
}

void Camera::SetPos(int button, int state, int x, int y) {
	//set camera position based on button
	if (button == 3 && state == GLUT_DOWN) {
		camera_position_delta += camera_up * .05f;
	} else if (button == 4 && state == GLUT_DOWN) {
		camera_position_delta -= camera_up * .05f;
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { //left mouse button is down
		move_camera = true; // move camera
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) { //left mouse button is up
		move_camera = false; // don't move camera
	}
	mouse_position = glm::vec3(x, y, 0);
}

CameraType Camera::GetMode() {
	return camera_mode; //retrieve the mode the camera is in
}

void Camera::GetViewport(int &loc_x, int &loc_y, int &width, int &height) {
	loc_x = viewport_x; 
	loc_y = viewport_y;
	width = window_width;
	height = window_height;
}

void Camera::GetMatricies(glm::mat4 &P, glm::mat4 &V, glm::mat4 &M) {
	P = projection;
	V = view;
	M = model;
}
