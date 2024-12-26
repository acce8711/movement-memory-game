
Overview of what you did
I made a memory movement game that tracks the movements using two colour tracked objects. The game begins with a pattern 
being shown and the player must memorize it. Then the player must repeat the movements in the same order as shown in the 
previous pattern. The available movements are in, out, further, and closer. Once the player makes a guess, and it is 
time to move to the next round, they will be prompted to collide their objects to continue. If at least one of the 
objects has lost detection, the player will be informed that they are not able to guess until both objects are detected. 
The program supports webcam and recorder video, toggles the debug view, tracks depth (and allows threshold depth adjustment).

What was challenging
The challenge was coding all of the game states, trying out background subtraction, and checking in the program how the 
movement of each object relates to each other. There were a lot of game states and there were small bugs when switching 
between them and it was oftentimes difficult to detect what was causing the bug. I had difficulties with background 
subtraction as I was not able to figure out how to differentiate between objects and track their collision. Also the 
background subtraction would track my whole hand which would make it difficult to detect the proper movements. It was 
difficult to figure out how the movement of each object relates to the other and when they both have the same movement.

What went well
What helped me figure out the bugs throughout each game state was using a lot of print statements and taking it one step at 
a time instead of trying to do everything at once. As for the background subtraction, I removed it completely as I was 
spending too much time on it and I thought it is better to invest that time in other areas of the project. For the movement 
coordination, I created several bool variables to track when both objects have been moved.


Sources:
Background image- https://unsplash.com/photos/cars-on-road-between-high-rise-buildings-during-night-time-yVUQlyRlJSw
Correct sound: https://pixabay.com/sound-effects/correct-156911/
Incorrect sound: https://pixabay.com/sound-effects/error-2-126514/
Background music: https://pixabay.com/sound-effects/wandering-6394/
Movement sound: https://pixabay.com/sound-effects/click-button-app-147358/	
Collision sound: https://pixabay.com/sound-effects/thump-87726/	
