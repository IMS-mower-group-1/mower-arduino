# mower-arduino

The encoder motor’s power is set to 50%, which is sufficient for the robot to avoid missing the line. If both line detector sensors fail to detect the line, the robot will move randomly at a random distance until it finds the line and resumes following it. Additionally, the ultrasonic sensor can detect barriers up to 10 cm away and will randomly change the robot’s direction in response to such obstacles.
