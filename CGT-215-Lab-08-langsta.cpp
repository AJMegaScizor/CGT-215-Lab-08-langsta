// BalloonBuster.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>
#include <vector>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;
using namespace sfp;

const float KB_SPEED = 0.2;
// prints an error if images cannot load
void LoadTex(Texture& tex, string filename) {
    if (!tex.loadFromFile(filename)) {
        cout << "Could not load " << filename << endl;
    }
}


// returns the location of text to help position
Vector2f GetTextSize(Text text) {
    FloatRect r = text.getGlobalBounds();
    return Vector2f(r.width, r.height);
}

int main()
{
    //creates window for program
    RenderWindow window(VideoMode(800, 600), "Duck Hunter");
    World world(Vector2f(0, 0));
    //tracks the values for score, arrow, and duck timer
    int score(0);
    int arrows(5);
    int duck_clock(0);
    
    // loads quack sfx
    SoundBuffer popBuffer;
    if (!popBuffer.loadFromFile("quack.ogg")) {
        cout << "could not load quack.ogg" << endl;
        exit(5);
    }
    // plays duck sound if a duck is hit
    Sound popSound;
    popSound.setBuffer(popBuffer);
    //loads music to play
    Music music;
    if (!music.openFromFile("circus.ogg")) {
        cout << "Failed to load circus.ogg ";
        exit(6);
    }
    music.play();
    //creates crossbow
    PhysicsSprite& crossBow = * new PhysicsSprite();
    Texture cbowTex;
    LoadTex(cbowTex, "images/crossbow.png");
    crossBow.setTexture(cbowTex);
    Vector2f sz = crossBow.getSize();
    crossBow.setCenter(Vector2f(400,
        600 - (sz.y / 2)));
    //creates arrows
    PhysicsSprite arrow;
    Texture arrowTex;
    LoadTex(arrowTex, "images/arrow.png");
    arrow.setTexture(arrowTex);
    bool drawingArrow(false);
    // invisible wall that is used to remove arrows on collision
    PhysicsRectangle top;
    top.setSize(Vector2f(800, 10));
    top.setCenter(Vector2f(400, 5));
    top.setStatic(true);
    world.AddPhysicsBody(top);
    // invisible wall used to remove ducks as they cross to the right side of the screen
    PhysicsRectangle right;
    right.setSize(Vector2f(10, 600));
    right.setCenter(Vector2f(795, 300));
    right.setStatic(true);
    world.AddPhysicsBody(right);
    //loads duck texture
    Texture redTex;
    LoadTex(redTex, "images/duck.png");
    PhysicsShapeList<PhysicsSprite> ducks;
    

    // removes arrows physics and drawing when it collides with the top of the screen
    top.onCollision = [&drawingArrow, &world, &arrow]
    (PhysicsBodyCollisionResult result) {
        drawingArrow = false;
        world.RemovePhysicsBody(arrow);
    };


   // loads arial font and notifies if it doesn't load
    Font fnt;
    if (!fnt.loadFromFile("arial.ttf")) {
        cout << "Could not load font." << endl;
        exit(3);
    }
    // sets up the clock system 
    Clock clock;
    Time lastTime(clock.getElapsedTime());
    Time currentTime(lastTime);

   // main game loop ends when arrows equal 0 or last arrow is still hasn't been removed
    while ((arrows>0)||drawingArrow) {
        currentTime = clock.getElapsedTime();
        Time deltaTime = currentTime - lastTime;
        long deltaMS = deltaTime.asMilliseconds();

    // tracks in-game time
        if (deltaMS > 9) {
            lastTime = currentTime;
            duck_clock += deltaMS;
            // references duck_clock variable to spawn a duck after every 500 milliseconds
            if (duck_clock >= 500) {
                duck_clock = 0;
                PhysicsSprite& duck = ducks.Create();
                duck.setTexture(redTex);
                Vector2f sz = duck.getSize();
                duck.setCenter(Vector2f(50, 20 + (sz.y / 2)));
                duck.setVelocity(Vector2f(0.25, 0));
                world.AddPhysicsBody(duck);
                // if a duck is shot by an arrow remove it from the game world and add 10 to the score
                duck.onCollision =
                    [&drawingArrow, &world, &arrow, &duck, &ducks, &score, &popSound, &right]
                    (PhysicsBodyCollisionResult result) {
                    if (result.object2 == arrow) {
                        popSound.play();
                        drawingArrow = false;
                        world.RemovePhysicsBody(arrow);
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                        score += 10;
                    }
                    // if duck hits the right invisible wall remove it from game space
                    if (result.object2 == right) {
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                    }
                    };
            }
            // updates physics
            world.UpdatePhysics(deltaMS);
            // if Space is pressed shoot an arrow and subtract 1 from the total count of available arrows
            if (Keyboard::isKeyPressed(Keyboard::Space) &&
                !drawingArrow) {
                drawingArrow = true;
                arrows = arrows - 1;
                arrow.setCenter(crossBow.getCenter());
                arrow.setVelocity(Vector2f(0, -1));
                world.AddPhysicsBody(arrow);
       
            }
          

            window.clear();
            // draw arrow if arrow is active
            if (drawingArrow) {
                window.draw(arrow);
            }
            // removes killed ducks
            ducks.DoRemovals();
            for (PhysicsShape& duck : ducks) {
                window.draw((PhysicsSprite&)duck);
            }
            // draws crossobw and text on screen
            window.draw(crossBow);
            Text scoreText;
            scoreText.setString(to_string(score));
            scoreText.setFont(fnt);
            scoreText.setPosition(Vector2f(790 - GetTextSize(scoreText).x, 565));
            window.draw(scoreText);
            Text arrowCountText;
            arrowCountText.setString(to_string(arrows));
            arrowCountText.setFont(fnt);
            arrowCountText.setPosition(Vector2f(15 - GetTextSize(arrowCountText).x, 565));
            window.draw(arrowCountText);
            //world.VisualizeAllBounds(window);

            window.display();
         
        }
    }
    window.display(); // this is needed to see the last frame
    // displays gameover message
    Text gameOverText;
    gameOverText.setString("GAME OVER");
    gameOverText.setFont(fnt);
    sz = GetTextSize(gameOverText);
    gameOverText.setPosition(400 - (sz.x / 2), 300 - (sz.y / 2));
    window.draw(gameOverText);
    window.display();
    // waits until space is released then if pressed again closes the window
    while (Keyboard::isKeyPressed(Keyboard::Space)) {

    }
    while (!Keyboard::isKeyPressed(Keyboard::Space)) {

    }
    window.close();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

