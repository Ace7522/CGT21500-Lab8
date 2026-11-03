#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>
#include <vector>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;
using namespace sfp;
const float KB_SPEED = 0.2;

static void LoadTex(Texture& tex, string filename)
{
    if (!tex.loadFromFile(filename))
    {
        cout << "Could not load " << filename << endl;
    }
}

static Vector2f GetTextSize(Text text)
{
    FloatRect r = text.getGlobalBounds();
    return Vector2f(r.width, r.height);
}

int main()
{
    // Initialize world and game variables
    RenderWindow window(VideoMode(800, 600), "Balloon Buster");
    World world(Vector2f(0, 0));
    int score(0);
    int arrows(5);

    // Create crossbow
    PhysicsSprite& crossBow = *new PhysicsSprite();
    Texture cbowTex;
    LoadTex(cbowTex, "Images/crossbow.png");
    crossBow.setTexture(cbowTex);
    Vector2f sz = crossBow.getSize();
    crossBow.setCenter(Vector2f(400, 600 - (sz.y / 2)));

    // Create arrows
    PhysicsSprite arrow;
    Texture arrowTex;
    LoadTex(arrowTex, "Images/arrow.png");
    arrow.setTexture(arrowTex);
    bool drawingArrow(false);

    // Draw boundaries
    PhysicsRectangle top;
    top.setSize(Vector2f(800, 10));
    top.setCenter(Vector2f(400, 5));
    top.setStatic(true);
    world.AddPhysicsBody(top);
    PhysicsRectangle right;
    right.setSize(Vector2f(10, 600));
    right.setCenter(Vector2f(795, 300));
    right.setStatic(true);
    world.AddPhysicsBody(right);

    // Load duck image
    Texture duckText;
    LoadTex(duckText, "Images/red_balloon.png");
    PhysicsShapeList<PhysicsSprite> ducks;

    // Lambda for spawning a duck
    auto spawnDuck = [&drawingArrow, &world, &arrow, &duckText, &ducks, &score, &right]()
        {
            PhysicsSprite& duck = ducks.Create();
            duck.setTexture(duckText);
            Vector2f sz = duck.getSize();
            duck.setCenter(Vector2f(0, 20 + (sz.y / 2)));
            duck.setVelocity(Vector2f(0.5, 0));
            world.AddPhysicsBody(duck);

            duck.onCollision = [&drawingArrow, &world, &arrow, &duck, &ducks, &score, &right]
                (PhysicsBodyCollisionResult result)
                {
                    if (result.object2 == arrow)
                    {
                        drawingArrow = false;
                        world.RemovePhysicsBody(arrow);
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                        score += 10;
                    }
                    else if (result.object2 == right)
                    {
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                    }
                };
        };

    // Remove arrows that hit the ceiling
    top.onCollision = [&drawingArrow, &world, &arrow]
        (PhysicsBodyCollisionResult result)
        {
        drawingArrow = false;
        world.RemovePhysicsBody(arrow);
        };

    // Initialize the font
    Font fnt;
    if (!fnt.loadFromFile("arial.ttf"))
    {
        cout << "Could not load font." << endl;
        exit(3);
    }

    // Initialize the clock
    Clock clock;
    Time lastTime(clock.getElapsedTime());
    Time currentTime(lastTime);
    int iterationTimer(0);

    // While theres at least 1 arrow or an arrow currently animating
    while ((arrows > 0) || drawingArrow)
    {
        // Get elapsed time
        currentTime = clock.getElapsedTime();
        Time deltaTime = currentTime - lastTime;
        long deltaMS = deltaTime.asMilliseconds();

        // Frame rate
        if (deltaMS > 9)
        {
            lastTime = currentTime;
            world.UpdatePhysics(deltaMS);

            // Spawn a duck every 50 frames
            if (iterationTimer == 0) spawnDuck();
            iterationTimer++;
            if (iterationTimer >= 50) iterationTimer = 0;

            // Shoot an arrow when the spacebar is pressed
            if (Keyboard::isKeyPressed(Keyboard::Space) && !drawingArrow)
            {
                drawingArrow = true;
                arrows = arrows - 1;
                arrow.setCenter(crossBow.getCenter());
                arrow.setVelocity(Vector2f(0, -1));
                world.AddPhysicsBody(arrow);

            }

            window.clear();

            if (drawingArrow) window.draw(arrow);

            ducks.DoRemovals();

            for (PhysicsShape& balloon : ducks)
            {
                window.draw((PhysicsSprite&)balloon);
            }

            window.draw(crossBow);

            // Draw arrow text
            Text arrowCountText;
            arrowCountText.setString(to_string(arrows));
            arrowCountText.setFont(fnt);
            arrowCountText.setPosition(Vector2f(5, 560));
            window.draw(arrowCountText);

            // Draw score text
            Text scoreText;
            scoreText.setString(to_string(score));
            scoreText.setFont(fnt);
            scoreText.setPosition(Vector2f(790 - GetTextSize(scoreText).x, 560));
            window.draw(scoreText);

            window.display();
        }
    }

    window.display();

    // Draw game over text
    Text gameOverText;
    gameOverText.setString("GAME OVER");
    gameOverText.setFont(fnt);
    sz = GetTextSize(gameOverText);
    gameOverText.setPosition(400 - (sz.x / 2), 300 - (sz.y / 2));
    window.draw(gameOverText);

    window.display();

    while (true);
}