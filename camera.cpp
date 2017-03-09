//camera.cpp
#include "camera.h" 
using namespace std;

Camera::Camera() {
	//INITALIZE CAMERA PROPERTIES
	camera_mode = FREE; //INITIALIZE CAMERA TO FREE MODE
	camera_up = glm::vec3(0, 1, 0); // INITIALIZE THE UP DIRECTION TO UP THE Y-AXIS
	field_of_view = 45; //SET FIELD OF VIEW TO 45
	camera_position_delta = glm::vec3(0, 0, 0); // CHANGE IN CAMERA POSITION IS 0
	camera_scale = .5f;
	max_pitch_rate = 5;
	max_heading_rate = 5;
	move_camera = false; // SET CAMERA STATIONARY
}
Camera::~Camera() {
}

void Camera::Reset() {
	//reset camera
	camera_up = glm::vec3(0, 1, 0); //SET UP DIRECTION TO UP THE Y-AXIS
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
		camera_heading *= .5; // HALF THE CAMERA HEADING
		camera_pitch *= .5; // HALF THE CAMERA PITCH
		camera_position_delta = camera_position_delta * .8f; // DECREASE CAMERA POSITION BY 20%
	}
	//compute the MVP
	view = glm::lookAt(camera_position, camera_look_at, camera_up); // SET THE VIEW TO THE GLUTFUCNTION LOOKAT
	model = glm::mat4(1.0f);
	MVP = projection * view * model;
}

//Setting Functions
void Camera::SetMode(CameraType cam_mode) {
	// SET CAMERA MODE
	camera_mode = cam_mode;
	camera_up = glm::vec3(0, 1, 0);  // SET UP DIRECTION TO UP THE Y-AXIS
}

void Camera::SetPosition(glm::vec3 pos) {
	// SET CAMERA POSITION
	camera_position = pos;
}

void Camera::SetLookAt(glm::vec3 pos) {
	// SET WHAT CAMERA LOOKS AT
	camera_look_at = pos;
}
void Camera::SetFOV(double fov) {
	// SET FIELD OF VIEW
	field_of_view = fov;
}
void Camera::SetViewport(int loc_x, int loc_y, int width, int height) {
	// SET VIEWPORT
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
void Camera::Move(CameraDirection dir) { // MOVE BASED ON PARAMETER
	if (camera_mode == FREE) { // IF CAMERA CAN MOVE
		switch (dir) { // GO TO CASE OF PARAMETER
			case UP: // IF PARAMTER IS UP START HERE
				camera_position_delta += camera_up * camera_scale; // MOVE CAMERA UP
				break; // EXIT SWITCH
			case DOWN: // IF PARAMTER IS DOWN START HERE
				camera_position_delta -= camera_up * camera_scale; // MOVE CAMERA DOWN
				break; // EXIT SWITCH
			case LEFT: // IF PARAMTER IS LEFT START HERE
				camera_position_delta -= glm::cross(camera_direction, camera_up) * camera_scale; // MOVE CAMERA LEFT
				break; // EXIT SWITCH
			case RIGHT: // IF PARAMTER IS RIGHT START HERE
				camera_position_delta += glm::cross(camera_direction, camera_up) * camera_scale; // MOVE CAMERA RIGHT
				break; // EXIT SWITCH
			case FORWARD: // IF PARAMETER IS FORWARD START HERE
				camera_position_delta += camera_direction * camera_scale; // MOVE CAMERA FORWARD
				break; // EXIT SWITCH
			case BACK: // IF PARAMTER IS BACK START HERE
				camera_position_delta -= camera_direction * camera_scale; // MOVE CAMERA BACK
				break; // EXIT SWITCH
		}
	}
}
void Camera::ChangePitch(float degrees) {
	//Check bounds with the max pitch rate so that we aren't moving too fast
	if (degrees < -max_pitch_rate) { // DEGREES MOVING TOO FAST IN NEGATIVE DIRECTION
		degrees = -max_pitch_rate; // CHANGE DEGREES TO MAX VALUE IN NEGATIVE DIRECTION
	} else if (degrees > max_pitch_rate) { // DEGEREES MOVING TOO FAST IN POSITIVE DIRECTION
		degrees = max_pitch_rate; // CHANGE DEGREES TO MAX VALUE IN POSITIVE DIRECTION
	}
	camera_pitch += degrees; // CHANGE CAMERA PITCH

	//Check bounds for the camera pitch
	if (camera_pitch > 360.0f) { // CAMERA PITCH GOES OVER 360 DEGREES IN POSITIVE DIRECTION
		camera_pitch -= 360.0f; // ROTATE CAMERA NEGATIVE 360 DEGREES TO NORMALIZE CAMERA_PITCH VALUE
	} else if (camera_pitch < -360.0f) { // CAMERA PITCH GOES OVER 360 DEGREES IN NEGATIVE DIRECTION
		camera_pitch += 360.0f; // ROTATE CAMERA POSITIVE 360 DEGREES TO NORMALIZE CAMERA_PITCH VALUE
	}
}
void Camera::ChangeHeading(float degrees) {
	//Check bounds with the max heading rate so that we aren't moving too fast
	if (degrees < -max_heading_rate) { // DEGREES MOVING TOO FAST IN NEGATIVE DIRECTION
		degrees = -max_heading_rate; // CHANGE DEGREES TO MAX VALUE IN NEGATIVE DIRECTION
	} else if (degrees > max_heading_rate) { // DEGEREES MOVING TOO FAST IN POSITIVE DIRECTION
		degrees = max_heading_rate; // CHANGE DEGREES TO MAX VALUE IN POSITIVE DIRECTION
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
	if (move_camera) { // CHECK IF MOVE_CAMERA IS TRUE
		ChangeHeading(.08f * mouse_delta.x);
		ChangePitch(.08f * mouse_delta.y);
	}
	mouse_position = glm::vec3(x, y, 0); // SET MOUSE POSITION ON THE X-Y PLANE BASED ON PARAMETERS
}

void Camera::SetPos(int button, int state, int x, int y) {
	// SET CAMERA POSITION BASED ON BUTTON
	if (button == 3 && state == GLUT_DOWN) { // SCROLL UP IS PRESSED DOWNW
		camera_position_delta += camera_up * .05f;
	} else if (button == 4 && state == GLUT_DOWN) { // SCROLL DOWN IS PRESSED DOWN
		camera_position_delta -= camera_up * .05f;
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) { // LEFT MOUSE BUTTON IS DOWN
		move_camera = true; // MOVE CAMERA
	} else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) { // LEFT MOUSE BUTTON IS UP
		move_camera = false; // DON'T MOVE CAMERA
	}
	mouse_position = glm::vec3(x, y, 0);
}

CameraType Camera::GetMode() {
	return camera_mode; //RETREIVE THE MODE THE CAMERA IS IN
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
