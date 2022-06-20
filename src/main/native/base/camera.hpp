/*
* Basic camera class
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once

#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Camera
{
private:
    
    
    
    
	void updateViewMatrix()
	{
		glm::mat4 rotM = glm::mat4(1.0f);
		glm::mat4 transM;

		rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        

		glm::vec3 translation = position;
		if (flipY) {
			translation.y *= -1.0f;
		}
        translation.z /= mZoom;
        
         
		transM = glm::translate(glm::mat4(1.0f), translation);
        matrices.view = transM * rotM;

 //       viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
        
		updated = true;
	};
public:

	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3();
    float mZoom = 1.0;


	bool updated = false;
	bool flipY = false;

	struct
	{
		glm::mat4 perspective;
		glm::mat4 view;
	} matrices;

 
	void setPerspective(float fov, float aspect, float znear, float zfar)
	{
		matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
		if (flipY) {
			matrices.perspective[1][1] *= -1.0f;
		}
	};
    
    void setOrthogonal(float left,
                       float right,
                       float bottom,
                       float top,
                       float zNear,
                       float zFar)
    {
        matrices.perspective = glm::ortho(left, right, bottom, top, zNear, zFar);
        if (flipY) {
            matrices.perspective[1][1] *= -1.0f;
        }

    }


	void setPosition(glm::vec3 position)
	{
		this->position = position;
		updateViewMatrix();
	}

	void setRotation(glm::vec3 rotation)
	{
		this->rotation = rotation;
		updateViewMatrix();
	}

	void rotate(glm::vec3 delta)
	{
		this->rotation += delta;
		updateViewMatrix();
	}

	void setTranslation(glm::vec3 translation)
	{
		this->position = translation;
		updateViewMatrix();
	};

	void translate(glm::vec3 delta)
	{
		this->position += delta;
		updateViewMatrix();
	}
    
    void setZoom(float zoom)
    {
        
    }
};

 
#endif  //  __CAMERA_HPP__
