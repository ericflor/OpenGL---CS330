How do I approach designing software?
When approaching software design, I start by understanding the problem or requirements thoroughly. This includes gathering all necessary information and identifying potential challenges or constraints.
I then create a high-level design or architecture, outlining the key components and their interactions. This helps in breaking down the complexity of the project.
Next, I consider user experience and interface design, focusing on usability and aesthetics to ensure the software is user-friendly.
Throughout the design process, I prioritize scalability, maintainability, and flexibility, aiming for a solution that can adapt to future changes or enhancements.
What new design skills has your work on the project helped you to craft?
While working on this project, I honed my skills in 3D modeling and rendering, which were essential for creating the 3D scene.
I also improved my skills in user interface design, ensuring a visually appealing and intuitive user experience within the 3D environment.
What design process did you follow for your project work?
My design process involved requirements gathering, followed by conceptualizing the 3D scene and its elements.
I created wireframes and sketches to plan the user interface and scene layout.
After receiving feedback, I iterated on the design, refining it based on user needs and project goals.
Once the design was finalized, I implemented it while keeping design principles and user feedback in mind.
How could tactics from your design approach be applied in future work?
The design tactics I employed in this project, such as user-centered design and iterative prototyping, can be applied to future projects. This ensures that I create solutions that are not only functional but also user-friendly and adaptable.
My experience with 3D modeling and rendering can be leveraged in projects that require similar visualizations or simulations.
How do I approach developing programs?
I approach program development by breaking down the problem into smaller, manageable tasks.
I choose appropriate programming languages and frameworks based on project requirements.
I write clean, modular, and well-documented code to ensure readability and maintainability.
Regular testing and debugging are crucial throughout the development process to catch and address issues early.
What new development strategies did you use while working on your 3D scene?
While working on the 3D scene, I employed graphics programming techniques to render objects and create interactive elements.
I used libraries and frameworks tailored to 3D graphics, allowing me to efficiently handle complex visualizations.
Optimization strategies were crucial to ensure smooth rendering and responsiveness in real-time 3D environments.
How did iteration factor into your development?
Iteration was a fundamental part of my development process. I regularly reviewed and tested the 3D scene, incorporating feedback and making improvements.
This iterative approach helped identify and address issues and led to an overall improvement in the quality of the 3D scene.
How has your approach to developing code evolved throughout the milestones, which led you to the project’s completion?
As I progressed through the project's milestones, I became more proficient in 3D graphics programming and improved my understanding of design patterns specific to 3D scenes.
My coding practices became more efficient, and I adopted better strategies for debugging and optimizing code.
I also gained a deeper appreciation for the importance of documentation in maintaining and extending code.
How can computer science help me in reaching my goals?
Computer science equips me with problem-solving skills that are essential in various fields and industries.
It provides the foundation for creating innovative software solutions and technology-driven advancements.
The analytical and logical thinking fostered by computer science can be applied to tackle complex challenges in my educational and professional endeavors.
How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future educational pathway?
Computational graphics and visualizations have enhanced my ability to communicate complex concepts and data effectively.
They provide a powerful means of conveying information, which can be valuable in educational settings for explaining abstract or scientific ideas.
These skills can be applied to create engaging educational materials or presentations.
How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future professional pathway?
Proficiency in computational graphics and visualizations can be a valuable asset in various professional fields, such as data analysis, architecture, game development, and simulation.
These skills enable me to create compelling visual representations of data and concepts, aiding decision-making and communication in a professional context.
Additionally, they can open up career opportunities in industries that rely on 3D modeling and visualizations for design, simulation, and virtual reality applications.


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
