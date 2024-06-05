# Lawn Mowing Platformer
#### Video Demo: 
#### Description: Mowing platformer with custom vehicle physics ( coded in C++ )

<p>One of the reasons I wanted to take this amazing course was to try my hand at game development, 
so for my final project, nothing else interested me more than trying to make a game! I decided I wanted to make a 3D game in Unreal using C++. 
Having no experience with either, I used the following resources before getting started:</p>

<a href="https://www.learncpp.com/" target="_blank">Learn C++ Tutorial Series</a><br>
<a href="https://www.udemy.com/course/unrealcourse/" target="_blank">Unreal Engine 5 C++ Developer: Learn C++ & Make Video Games Course</a><br>
<a href="https://www.youtube.com/playlist?list=PLjEaoINr3zgEPv5y--4MKpciLaoQYZB1Z" target="_blank">Blender Guru’s Blender 4.0 Donut Tutorial</a>

<p>Nearly 3 months later, I felt that I was ready to make my lawn mowing game!</p>

<strong><span style="color:green">The Lawn Mower</span></strong>

<p>The first thing I focused on was making my player character, the lawn mower. 
I wanted my lawn mower to have exaggerated vehicle physics akin to games like Rocket League. 
My first attempt at a physics enabled vehicle was inspired by the following tutorial:</p>

<a href="https://www.youtube.com/watch?v=PWTCM6sZ6OE" target="_blank">UE4 Car Vehicle Wheel with Suspension using Physics: 00:00 - 11:58</a>

<p>I created a similar vehicle class as seen in the video ( Mower.cpp and Mower.h ) and gave it the ability to accelerate, 
reverse, brake, steer and even jump, but it had a glaring issue that I could not ignore. The way that the wheels were constrained 
to the lawn mower body meant that there was no way to steer the wheels in the yaw direction and also rotate them in the pitch direction, 
it was one or the other. So I went back to the drawing board to see if there was another way I could construct my vehicle from scratch 
that allowed me to animate the wheels and still use physics.</p>

<p>I ended up finding the following video, where a developer used raycasts (referred to as traces in Unreal) 
on the corners of a physics rigid body to look for ground and apply force upward onto the vehicle to keep it hovering in the air, 
then use other forces and torque to move it and animate the meshes separately:</p>

<a href="https://www.youtube.com/watch?v=LG1CtlFRmpU" target="_blank">Space Dust Racing UE4 Arcade Vehicle Physics Tour</a>

<p>Before making my mower hover off the ground, I needed to know how much force to apply upward to oppose gravity, 
which for Unreal turned out to be the mass of the rigid body multiplied by 980 for gravitational acceleration. 
Now that my mower could float, I noticed that if I moved or rotated the mower, it would maintain constant linear or angular velocity. 
That’s when I discovered the second key to hovering, Linear and Angular damping. These member variables are part of the UPrimitiveComponent class, 
which is a parent class of the rigid body component (UBoxComponent) on my mower. Increasing these variables’ values 
caused the floating mower to slow down after being moved or rotated by applying a sort of friction.</p>

<p>For the starting point of my traces, I initialized const FVector ( x, y, z ) member variables storing positions 
relative to the local space of my new mower class ( MowerRC.cpp and MowerRC.h ). Then I defined two structs, FRayCastGroup and FLocalOrigins, 
to more easily pass the FHitResult member variables ( used to store information about what the trace hits) and the trace starting point FVectors 
into the functions where I call the line trace for each corner of the mower. Using Transform calls, the FVectors passed in could be 
used to initialize new FVectors that relate to those starting points on the mower, but in world space. Just as in the video, 
since there are four points at which force is applied, the amount of force required to oppose gravity divided by four would mean 
that the compression of the raycasts should be around 0.25 of 1.0.</p>

<p>So now the mower can float, but it was not stable until I applied linear and angular damping based on the compression of the traces 
and how many traces were hitting the ground. I did this by storing the float values of successful traces in dynamic arrays, 
(a TArray specifically, the Unreal equivalent of the standard library dynamic array class std::vector) one for linear damping 
and one for angular damping, so I could add up those floats later using a for each loop and apply the correct amount of damping each frame. 
After some tinkering I found values that worked well and now I had a hovering mower! But that was only the beginning of the mower development journey, 
and I went on to add a bunch of functionality for movement, and eventually my wheel animations!</p>

<p>I found that using member variables to represent local space FVectors and FRotators ( pitch, yaw, roll ), learning how to use transforms, 
inverse transforms ( in the case of my custom camera movement ), traces, and control non-const member variable values each Tick 
( the function called each frame ) were important to my implementation success.</p> 

<p>Throughout the development of my mower class, which went through many refactors, I tried to give my functions and variables more descriptive names 
for readability and organize my code files, so that the functions and variables were listed in the order they were called each frame. 
This made it easier for me to follow and recall what each function was doing without needing a ton of comments. Also, replacing const variables 
in cpp function definitions with const member variables in my header, allowed me to quickly tweak, compile and test changes, without needing 
to jump around my code while tuning a new feature.</p> 

<strong><span style="color:green">Spawning Grass</span></strong>

<p>The second biggest challenge I had before me was that I wanted a way to generate the grass at runtime using a blue noise pattern. 
I learned about this concept of grass generation from the following video:</p>

<a href="https://www.youtube.com/watch?v=Ge3aKEmZcqY&t=2438s" target="_blank">Simple Code, High Performance 40:38 - 46:13</a>
