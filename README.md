# Lawn Mowing Platformer
#### Video Demo: 
#### Description: Mowing platformer with custom vehicle physics (coded in C++)

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

<p>I created a similar vehicle class as seen in the video (Mower.cpp / Mower.h) and gave it the ability to accelerate, 
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

<p>For the starting point of my traces, I initialized const FVector (x, y, z) member variables storing positions 
relative to the local space of my new mower class (MowerRC.cpp / MowerRC.h). Then I defined two structs, FRayCastGroup and FLocalOrigins, 
to more easily pass the FHitResult member variables (used to store information about what the trace hits) and the trace starting point FVectors 
into the functions where I call a line trace for each corner of the mower. Using Transform calls, the FVectors passed in could be 
used to initialize new FVectors that relate to those starting points on the mower, but in world space. Just as in the video, 
since there are four points at which force is applied, the amount of force required to oppose gravity divided by four would mean 
that the compression of the raycasts should be around 0.25 of 1.0.</p>

<p>So now the mower could float, but it was not stable until I applied linear and angular damping based on the compression of the traces 
and how many were hitting the ground. I did this by storing the float values of successful traces in dynamic arrays, 
(TArrays specifically, the Unreal equivalent of the standard library dynamic array class std::vector) one for linear damping 
and one for angular damping, so I could add up those floats later using a for each loop and apply the correct amount of damping each frame. 
After some tinkering, I found values that worked well and now I had a hovering mower! But that was only the beginning of the mower development journey, 
and I went on to add a bunch of functionality for movement by applying force to the rigid body, and eventually my wheel animations!</p>

<p>I found that learning how to take advantage of FVector and FRotator (pitch, yaw, roll) member variables to represent local space values, transforms 
and inverse transforms (in the case of my custom camera movement), traces, and Tick order (the function called each frame for an actor object in the world)
were important to my development success.</p>

<p>Throughout the development of my mower class, which went through many refactors, I tried to give my functions and variables more descriptive names 
for readability and organize my code files, so that the functions and variables were listed in the order they were called or referenced each frame. 
This made it easier for me to follow and recall what each function was doing without needing a ton of comments. Also, replacing const variables 
in function definitions with const member variables in my header, allowed me to quickly tweak, compile and test changes, without needing 
to jump around my code while tuning a new feature.</p>

<strong><span style="color:green">Spawning Grass</span></strong>

<p>The second biggest challenge I had before me was that I wanted a way to generate the grass at runtime as an expanding blue-noise wave pattern. 
I learned about this concept of grass generation from the following video:</p>

<a href="https://www.youtube.com/watch?v=Ge3aKEmZcqY&t=2438s" target="_blank">Simple Code, High Performance 40:38 - 46:13</a>

<p>One of the most important things to consider for my idea was spawning grass on angles correctly, which ideally I wanted to do using something 
like a curved trace, where I could use the impact normal (a vector in the opposite direction of a trace hit) to inform the rotation of new grass. 
But unfortunately, I could not find any such member function in the UWorld class.</p>

<p>So I came up with the idea of using the same trace I used for the mower hovering (and later the wheel suspension animations) at various angles 
to look for ground, do a sweep (a trace using an object like a sphere) for other grass, then spawn new grass if hitting ground but not grass. 
I imagined it like a hammer swinging down, and when it struck the ground, it would spawn new grass if there wasn’t any already at that location.</p> 

<p>First, I made a grass spawner to test that I could spawn grass blades when hitting ground but not grass (GrassSpawner.cpp / GrassSpawner.h). 
This ended up operating like a pen that I could use to draw grass onto the ground. Now that I had the spawning realized, it was time to make 
my grass spawning class (Grass.cpp / Grass.h)</p>

<p>For the hammer swing, I needed to use something to represent the rotation of the hammer and the direction of the hammer’s head, so I used two 
USceneComponent class objects, constructed at runtime (so that I could destroy them after the spawning process to free their allocated memory).</p>

<p>One of these USceneComponents I named the Spawner, the hammer’s head, which I would use to get the downward trace direction, by using a flipped 
up vector that was transformed from the local space of the USceneComponent to world space. The other, I named the Rotator, which would change 
pitch by -22.5 degrees each frame, and change yaw by +60 degrees each time the hammer completed a swing. The range of a swing’s pitch was from 
67.5 degrees to -67.5 degrees, so the hammer had 7 pitch positions and 6 yaw positions. And since I attached the Spawner to the Rotator, when the
rotation of Rotator was changed, the Spawner would follow it. The entire process of hammer swings for one blade of grass, 
that could not find ground or ground where grass did not exist, had an upper-bound of 42 frames. This had two problems, one it was pretty slow 
and two the overlapping calculations of too many of these hammers swinging at once impacted the performance greatly!</p>

<p>I updated the code to do a trace for grass before the trace for ground at each pitch angle and cancel a swing early if grass was hit, rather than 
waiting for a ground hit to check for grass. This helped some with the processing of the calculations, but I also noticed that grass would 
spawn over hard edges, because of the range of the swing. So I needed a way to detect if a hammer swing was near a steep edge, if I wanted it
to still spawn on angles correctly.</p>

<p>Besides performance issues with tracing and spawning, another problem was the impact on the renderer to draw thousands of blades on screen at one time. 
Through research and testing, I found the biggest improvement to rendering performance by enabling Nanite on my grass blade mesh, and disabling 
Unreal’s lighting feature Lumen for my project. Then I found that I might get even better rendering performance if my grass meshes were static 
instead of movable.</p>

<p>So with all of the issues, I developed a newer version of my Grass spawning class (GrassB.cpp / GrassB.h) with a shorter pitch range, an edge detection 
trace on the first pitch position of each swing to cancel early if ground was not hit at a long distance, a mesh component that is created, attached, 
has its transform randomized, and set to be static after instantiation, and a randomized yaw position for the Rotator so spawn placement looks more natural. 
Now the upper-bound is 30 frames, or 6 frames if the object is surrounded by grass or cannot find ground! Even with this better version however, 
I still experience poor performance when so many are hammering away. So in the future, I have a plan to build an even better version with a lower 
and upper-bound of 1 frame!</p>

<strong><span style="color:green">Gameplay</span></strong>