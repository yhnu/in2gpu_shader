
#include "lab_mesh_loader.hpp"
#include "lab_geometry.hpp"
#include "lab_shader_loader.hpp"
#include "lab_glut.hpp"
#include "lab_texture_loader.hpp"
#include "lab_framebuffer.hpp"
#include <ctime>


class Laborator : public lab::glut::WindowListener{


private:
	unsigned int screen_width, screen_height;
	const int Trees = 230;
	glm::mat4 view_matrix, projection_matrix;			//object model matrix
	glm::mat4 model_matrx[230];
	glm::mat4 sun_model_matrix;
	glm::mat4 diffuse_model_matrix;
	glm::mat4 ground_model_matrix;

	unsigned int gl_program_shader_rim;									//id obiect shader rim
	unsigned int gl_program_shader_phong;							    //id obiect shader phong(for sun)
	unsigned int rtt_program_shader;									//render to texture 


	unsigned int gl_program_shader_curent;									//id obiect shader curent
	unsigned int mesh_vbo, mesh_ibo, mesh_vao, mesh_num_indices;			//containers 
	unsigned int mesh_vbo2, mesh_ibo2, mesh_vao2, mesh_num_indices2;		//containers 
	unsigned int mesh_vbo3, mesh_ibo3, mesh_vao3, mesh_num_indices3;		//containers 
	unsigned int ground_vbo, ground_ibo, ground_vao, ground_num_indices;	//containers 
	unsigned int rtt_vbo, rtt_ibo, rtt_vao, rtt_num_indices;				//geometrie suport pentru render-to-texture 
	//in world coords
	glm::vec3 light_position;
	glm::vec3 eye_position;
	glm::vec3 light_position_sun;

	lab::Framebuffer fbo; //our fbo

	unsigned int tree1_texture_color;
	unsigned int tree7_texture_color;
	unsigned int ground_texture;
    
	//used in sun shader
	float material_kd;

public:
	
	
	Laborator(){
		
		glClearColor(0.05,0.03,0.00,1);
		glClearDepth(1);			
		glEnable(GL_DEPTH_TEST);	

		//load shaders
		gl_program_shader_rim = lab::loadShader("shadere\\shader_rim_vertex.glsl", "shadere\\shader_rim_fragment.glsl");
		gl_program_shader_phong = lab::loadShader("shadere\\shader_phong_vertex.glsl", "shadere\\shader_phong_fragment.glsl");
		gl_program_shader_phong = lab::loadShader("shadere\\shader_phong_vertex.glsl", "shadere\\shader_phong_fragment.glsl");
		
		//set current shader
		gl_program_shader_curent = gl_program_shader_rim;
		
		//load shader
		lab::loadObj("resurse\\tree1_trunk.obj",mesh_vao, mesh_vbo, mesh_ibo, mesh_num_indices);
		lab::loadObj("resurse\\tree7_trunk.obj", mesh_vao3, mesh_vbo3, mesh_ibo3, mesh_num_indices3);
		lab::loadObj("resurse\\sphere.obj", mesh_vao2, mesh_vbo2, mesh_ibo2, mesh_num_indices2);
	
		tree1_texture_color = lab::loadTextureBMP("resurse\\trunk_color.bmp");
		tree7_texture_color = lab::loadTextureBMP("resurse\\trunk7_color.bmp");

		eye_position = glm::vec3(0,0,95);
		light_position = glm::vec3(50,0,50);
		light_position_sun = glm::vec3(30, -10, 50);

		material_kd = 0.5;

		//ground 
		lab::loadObj("resurse\\ground.obj", ground_vao, ground_vbo, ground_ibo, ground_num_indices);
		ground_texture = lab::loadTextureBMP("resurse\\ground.bmp");
		ground_model_matrix = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 10, -25, 0, 1);
		
		diffuse_model_matrix = glm::mat4(0.1, 0, 0, 0, 0, 0.1, 0, 0, 0, 0, 0.1, 0, 50, 0, 50, 1);
		int v;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &v);

		int j = 0;
		int k = 0;
		for (int i = 0; i < Trees/2; i++){

			if (i % 10 == 0)
			{
				j = 0;
				k++;
			}
			float	r = ((double)rand() / (RAND_MAX));
			model_matrx[i] = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, (4+ r) * j, -10, (7 + r) * k, 1);
			j++;
		}
		k = 0;
		for (int i = Trees / 2; i < Trees; i++){
			if (i % 10 == 0)
			{
				j = 0;
				k++;
			}
			float	r = ((double)rand() / (RAND_MAX));
			model_matrx[i] = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, (4 - r) * j, -10, (7 + r) * k, 1);
			j--;
		}

		sun_model_matrix = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 70, -160, 1);
		view_matrix = glm::lookAt(eye_position, glm::vec3(0,0,0), glm::vec3(0,1,0));

		rtt_program_shader = lab::loadShader("shadere\\shader_rtt_vertex.glsl", "shadere\\shader_rtt_fragment.glsl");

		struct RTT_Vertex{
			glm::vec3 position;
			glm::vec2 texcoord;
			RTT_Vertex(const glm::vec3 &pos, const glm::vec2 &tc){ position = pos; texcoord = tc; }
		};
		glGenVertexArrays(1, &rtt_vao);
		glBindVertexArray(rtt_vao);

	
		std::vector<RTT_Vertex> vertices; //4 vertecsi in coordonate NDC
		vertices.push_back(RTT_Vertex(glm::vec3(-1, -1, 0), glm::vec2(0, 0)));
		vertices.push_back(RTT_Vertex(glm::vec3( 1, -1, 0), glm::vec2(1, 0)));
		vertices.push_back(RTT_Vertex(glm::vec3( 1,  1, 0), glm::vec2(1, 1)));
		vertices.push_back(RTT_Vertex(glm::vec3(-1,  1, 0), glm::vec2(0, 1)));
		std::vector<unsigned int> indices;
		indices.push_back(0); indices.push_back(1); indices.push_back(2);
		indices.push_back(2); indices.push_back(3); indices.push_back(0);	

		glGenBuffers(1, &rtt_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, rtt_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(RTT_Vertex) * 4, &vertices[0], GL_STATIC_DRAW);
		glGenBuffers(1, &rtt_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rtt_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RTT_Vertex), (void*)0);
		glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RTT_Vertex), (void*)sizeof(glm::vec3));
		rtt_num_indices = 6;


		//draw
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	~Laborator(){
		
		glDeleteProgram(gl_program_shader_rim);
		glDeleteProgram(gl_program_shader_phong);
		glDeleteProgram(rtt_program_shader);
		glDeleteProgram(gl_program_shader_curent);
		
		glDeleteBuffers(1,&mesh_vbo);
		glDeleteBuffers(1,&mesh_ibo);
		glDeleteVertexArrays(1,&mesh_vao);

		glDeleteBuffers(1, &ground_vbo);
		glDeleteBuffers(1, &ground_ibo);
		glDeleteVertexArrays(1, &ground_vao);
		glDeleteTextures(1, &ground_texture);
		glDeleteTextures(1, &tree1_texture_color);		
		glDeleteTextures(1, &tree7_texture_color);

		glDeleteProgram(rtt_program_shader);
		
	}


		void notifyBeginFrame(){
		//rotatie light on Oy axis
				
	}
	float random(float low, float high) {
			return rand() / (high - low + 1) + low;
	}
	void notifyDisplayFrame(){
		
		//PASS1 Scene -> framebuffer
		//bind to fbo
		fbo.bind();
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Sun
			glUseProgram(gl_program_shader_phong);
			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader_phong, "model_matrix"), 1, false, glm::value_ptr(sun_model_matrix));
			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader_phong, "view_matrix"), 1, false, glm::value_ptr(view_matrix));
			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader_phong, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));

			glUniform3f(glGetUniformLocation(gl_program_shader_phong, "light_position"), light_position_sun.x, light_position_sun.y, light_position_sun.z);
			glUniform3f(glGetUniformLocation(gl_program_shader_phong, "eye_position"), eye_position.x, eye_position.y, eye_position.z);
			glUniform1f(glGetUniformLocation(gl_program_shader_phong, "material_kd"), material_kd);

			//bind obiect
			glBindVertexArray(mesh_vao2);
			glDrawElements(GL_TRIANGLES, mesh_num_indices2, GL_UNSIGNED_INT, 0);

			//Rim Lighting
			glUseProgram(gl_program_shader_curent);

			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader_curent, "view_matrix"), 1, false, glm::value_ptr(view_matrix));
			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader_curent, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));

			glUniform3f(glGetUniformLocation(gl_program_shader_curent, "light_position"), light_position.x, light_position.y, light_position.z);
			glUniform3f(glGetUniformLocation(gl_program_shader_curent, "eye_position"), eye_position.x, eye_position.y, eye_position.z);

			glUniformMatrix4fv(glGetUniformLocation(gl_program_shader_curent, "model_matrix"), 1, false, glm::value_ptr(ground_model_matrix));


			//ground
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ground_texture);
			glUniform1i(glGetUniformLocation(gl_program_shader_curent, "texture"), 0);


			glBindVertexArray(ground_vao);
			glDrawElements(GL_TRIANGLES, ground_num_indices, GL_UNSIGNED_INT, 0);

			for (int i = 0; i < Trees; i++){

				//send vars
				glUniformMatrix4fv(glGetUniformLocation(gl_program_shader_curent, "model_matrix"), 1, false, glm::value_ptr(model_matrx[i]));

				if (i % 2 == 0)
				{

					glBindTexture(GL_TEXTURE_2D, tree1_texture_color);
					glBindVertexArray(mesh_vao);
					glDrawElements(GL_TRIANGLES, mesh_num_indices, GL_UNSIGNED_INT, 0);

				}
				else
				{

					glBindTexture(GL_TEXTURE_2D, tree7_texture_color);
					glBindVertexArray(mesh_vao3);
					glDrawElements(GL_TRIANGLES, mesh_num_indices3, GL_UNSIGNED_INT, 0);
				}
			}
		}
		//unbind fbo, bind OpenGL's FBO
		fbo.unbind();


		//PASS 2 
		//draw the quad (4 points)
		{
			//clear screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(rtt_program_shader);

			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, fbo.getColorTexture());
			glUniform1i(glGetUniformLocation(rtt_program_shader, "texture_color"), 1);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, fbo.getDepthTexture());
			glUniform1i(glGetUniformLocation(rtt_program_shader, "texture_depth"), 2);
			glUniform1i(glGetUniformLocation(rtt_program_shader, "screen_width"), screen_width);
			glUniform1i(glGetUniformLocation(rtt_program_shader, "screen_height"), screen_height);

			//c is a control variable - change it manually otherwise implement it :)
			//0 is blur 
			//1 is depth
			//2 is color texture
			glUniform1i(glGetUniformLocation(rtt_program_shader, "c"), 1);
			glBindVertexArray(rtt_vao);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

	}
	
	void notifyEndFrame(){}
	
	void notifyReshape(int width, int height, int previos_width, int previous_height){
		//reshape
		if(height==0) height=1;

		screen_height = height;	
		screen_width = width;

		glViewport(0,0,width,height);
		projection_matrix = glm::perspective(90.0f, (float)width/(float)height,0.1f, 10000.0f);
		fbo.GenerateFBO(width, height);
	
	}

		
	
	//keyboard 
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == 27) lab::glut::close();	//ESC close glut
		if(key_pressed == 32) {
			//SPACE to reload shader
			glDeleteProgram(gl_program_shader_rim);
			glDeleteProgram(gl_program_shader_phong);

			gl_program_shader_rim = lab::loadShader("shadere\\shader_rim_vertex.glsl", "shadere\\shader_rim_fragment.glsl");
			gl_program_shader_phong = lab::loadShader("shadere\\shader_phong_vertex.glsl", "shadere\\shader_phong_fragment.glsl");

			gl_program_shader_curent = gl_program_shader_rim;
		}
		if(key_pressed == 'w'){
			static bool wire =true;
			wire=!wire;
			glPolygonMode(GL_FRONT_AND_BACK, (wire?GL_LINE:GL_FILL));
		}
		if(key_pressed == '1'){
			gl_program_shader_curent = gl_program_shader_rim;
		}
		
	}
	//tasta ridicata
	void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y){	}
	//tasta speciala (up/down/F1/F2..) apasata
	void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == GLUT_KEY_F1) lab::glut::enterFullscreen();
		if(key_pressed == GLUT_KEY_F2) lab::glut::exitFullscreen();
	}
	//tasta speciala ridicata
	void notifySpecialKeyReleased(int key_released, int mouse_x, int mouse_y){}
	//drag cu mouse-ul
	void notifyMouseDrag(int mouse_x, int mouse_y){ }
	//am miscat mouseul (fara sa apas vreun buton)
	void notifyMouseMove(int mouse_x, int mouse_y){ }
	//am apasat pe un boton
	void notifyMouseClick(int button, int state, int mouse_x, int mouse_y){ }
	//scroll cu mouse-ul
	void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y){ std::cout<<"Mouse scroll"<<std::endl;}

};

int main(){
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("lab shadere 1"),800,600,100,100,true);
	lab::glut::ContextInfo context(3,3,false);
	lab::glut::FramebufferInfo framebuffer(true,true,true,true);
	lab::glut::init(window,context, framebuffer);

	//initializeaza GLEW (ne incarca functiile openGL, altfel ar trebui sa facem asta manual!)
	glewExperimental = true;
	glewInit();
	std::cout<<"GLEW:initializare"<<std::endl;

	//creem clasa noastra si o punem sa asculte evenimentele de la GLUT
	//DUPA GLEW!!! ca sa avem functiile de OpenGL incarcate inainte sa ii fie apelat constructorul (care creeaza obiecte OpenGL)
	Laborator mylab;
	lab::glut::setListener(&mylab);

	std::cout << "Press 1 for Gouraud" << std::endl;
	std::cout << "Press 2 for Phong" << std::endl;
	std::cout << "Press 3 for Blinn-Phong" << std::endl;
	std::cout << "Press 4 for Phong with attenuation" << std::endl;
	std::cout << "Press Space to reload" << std::endl;
	std::cout << "Press w for wireframe" << std::endl;
	//run
	lab::glut::run();

	return 0;
}