


Reflection on Development Choices for 3D Swimming Pool Courtyard Scene

The decision to incorporate a swimming pool and walkway into the 3D scene was driven by the aim to create a tranquil and visually appealing environment. The pool, being a central element, required a realistic appearance with ripple effects to emulate water, achieved by using a ripple texture. The walkway, textured with a tiled brick pattern, adds contrast and leads the viewer's eye through the scene, enhancing the overall composition.
Programming for Required Functionality
To achieve the required functionalities:
1.	Lighting: Implemented Phong lighting model with ambient, diffuse, and specular components to simulate realistic lighting conditions. The inclusion of a spotlight provided focused illumination, adding depth and dimension to the scene.
2.	Textures: High-resolution textures were sourced for the pool (water ripple) and walkway (brick pattern). These textures were mapped to the respective 3D models to enhance realism.
3.	Projection: A toggle between perspective and orthographic projection was implemented, allowing users to switch views and gain different perspectives of the 3D environment.
User Navigation in the 3D Scene
Navigation through the scene is facilitated by a combination of keyboard and mouse inputs:
•	WASD keys: Control forward, backward, left, and right movements.
•	QE keys: Enable upward and downward movement.
•	Mouse cursor: Changes the orientation of the camera for a full view of the scene.
•	Mouse scroll: Adjusts the movement speed, providing a controlled exploration experience.
Explanation of Custom Functions
1.	UCreatePool, UCreateWalkway, UCreateCube: These functions create the mesh for the pool, walkway, and tables respectively. They are reusable for any similar object creation, reducing code redundancy.
2.	mouse_callback, scroll_callback: These handle mouse movements and scrolling, updating the camera's orientation and speed. They encapsulate the camera control logic, making the code modular.
3.	UInitialize, URender: UInitialize sets up the GLFW and GLEW libraries and creates the window. URender is called in the main loop to render the scene. Both functions demonstrate separation of concerns, with URender being particularly reusable for any OpenGL rendering task.
4.	UCreateShaderProgram: Compiles and links vertex and fragment shaders. This function is versatile and can be used with different shaders for various rendering effects.
5.	loadTexture: A utility function to load textures from files, demonstrating code reuse for any texture loading requirements in OpenGL.
These functions collectively contribute to a clean, maintainable, and modular code structure, crucial for any scalable and complex 3D graphics application.
![image](https://github.com/ericflor/OpenGL---CS330/assets/46897396/f6163a41-1327-4000-b20a-aeb87225fdf6)
