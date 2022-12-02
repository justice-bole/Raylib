#include <iostream>

#include <raylib.h>
#include <array>
#include <fstream>
#include <vector>

#include "Window.h"
#include "Player.h"
#include "Racer.h"

enum class GameState {
    logo,
    title,
    game,
    end,
    maxstates,
};

void checkMove(Player& player, int columns);
void drawRoad(int winHeight, int winWidth, int halfHeight, int halfWidth , Color&& color);
void drawRoadMarkers(int winHeight, int winWidth, int halfHeight, int halfWidth , Color&& color);
void controlRacer(Racer& racer, GameState& currentState, Player& player, int& score);
void resetRacer(Racer& racer);

int main() {

    //Window settings
    constexpr int winWidth{960};
    constexpr int winHeight{static_cast<int>(winWidth * .75f)}; //we can change the winWidth and keep the same aspect ratio
    Window window{winWidth, winHeight, "Dodger"};
    window.initialize();
    Window::setDefaultFps(); //sets fps to monitors max refresh rate

    constexpr int halfWidth{static_cast<int>(winWidth * 0.5f)};
    constexpr int halfHeight{static_cast<int>(winHeight * 0.5f)};

    constexpr int columns = 5;
    constexpr int rows = 5;

    //Audio settings
    InitAudioDevice();

    std::vector<Music> tracks{};
    std::string musicPath{"Music"};

    //Load music into vector
    for (const auto& entry : std::filesystem::directory_iterator(musicPath)) {
        tracks.push_back(LoadMusicStream(entry.path().c_str()));
    }

    int currentTrack{GetRandomValue(0, static_cast<int>(tracks.size() - 1))};
    PlayMusicStream(tracks[currentTrack]);
    float timePlayed{0};

    //Shape settings
    constexpr int rectWidth{winWidth / columns};
    constexpr int rectHeight{halfHeight / rows};

    //Player settings
    constexpr int startRow{1};
    constexpr int startColumn{2};
    Rectangle playerRect{rectWidth * startColumn, winHeight - (rectHeight * startRow), rectWidth, rectHeight};
    Player player{playerRect, DARKBLUE};

    //Racer settings
    float row2Width{rectWidth * .75f};
    float row3Width{rectWidth *.50f};
    float row4Width{rectWidth *.25f};
    float row5Width{rectWidth *.12};

    float row2Height{rectHeight *.75f};
    float row3Height{rectHeight *.50f};
    float row4Height{rectHeight *.25f};
    float row5Height{rectHeight *.12f};

    //closest to furthest
    Rectangle leftRacer1{rectWidth * .5f, winHeight - (rectHeight * 1), rectWidth, rectHeight};
    Rectangle leftRacer2{rectWidth * 1, winHeight - (rectHeight * 2), row2Width, row2Height};
    Rectangle leftRacer3{rectWidth * 1.5, winHeight - (rectHeight * 3), row3Width, row3Height};
    Rectangle leftRacer4{rectWidth * 2, winHeight - (rectHeight * 4), row4Width, row4Height};
    Rectangle leftRacer5{rectWidth * 2.25f, winHeight - (rectHeight * 4.5), row5Width, row5Height};

    Rectangle middleRacer1{rectWidth * 2, winHeight - (rectHeight * 1), rectWidth, rectHeight};
    Rectangle middleRacer2{rectWidth * 2.12, winHeight - (rectHeight * 2), row2Width, row2Height};
    Rectangle middleRacer3{rectWidth * 2.25, winHeight - (rectHeight * 3), row3Width, row3Height};
    Rectangle middleRacer4{rectWidth * 2.37, winHeight - (rectHeight * 4), row4Width, row4Height};
    Rectangle middleRacer5{rectWidth * 2.44f, winHeight - (rectHeight * 4.5), row5Width, row5Height};

    Rectangle rightRacer1{rectWidth * 3.5f, winHeight - (rectHeight * 1), rectWidth, rectHeight};
    Rectangle rightRacer2{rectWidth * 3.25, winHeight - (rectHeight * 2), row2Width, row2Height};
    Rectangle rightRacer3{rectWidth * 3.0, winHeight - (rectHeight * 3), row3Width, row3Height};
    Rectangle rightRacer4{rectWidth * 2.75, winHeight - (rectHeight * 4), row4Width, row4Height};
    Rectangle rightRacer5{rectWidth * 2.63f, winHeight - (rectHeight * 4.5), row5Width, row5Height};

    std::array<Rectangle, 5> leftRacerRecs{leftRacer5, leftRacer4, leftRacer3, leftRacer2, leftRacer1};
    std::array<Rectangle, 5> middleRacerRecs{middleRacer5, middleRacer4, middleRacer3, middleRacer2, middleRacer1};
    std::array<Rectangle, 5> rightRacerRecs{rightRacer5, rightRacer4, rightRacer3, rightRacer2, rightRacer1};
    std::array<std::array<Rectangle , 5>, 3> allRacerRecs{leftRacerRecs, middleRacerRecs, rightRacerRecs};

    int racerPosition{0};

    //frame timings for movement
    int racerMoveCooldown{0};

    int baseRacerUpdateInterval{60};
    int minSpeed{10};
    int racerUpdateInterval{baseRacerUpdateInterval};

    int timeToSpawn{baseRacerUpdateInterval * 2};
    int spawnTime{timeToSpawn};

    bool racerCamSpawn = false;

    Color racerColor{BLUE};

    Racer racerL{allRacerRecs, RacerType::leftRacer, racerMoveCooldown, racerUpdateInterval, racerPosition, racerCamSpawn, racerColor};
    Racer racerM{allRacerRecs, RacerType::midRacer, racerMoveCooldown, racerUpdateInterval, racerPosition, racerCamSpawn, racerColor};
    Racer racerR{allRacerRecs, RacerType::rightRacer, racerMoveCooldown, racerUpdateInterval, racerPosition, racerCamSpawn, racerColor};

    //player stats
    int score{0};
    int highscore{0};
    int distance{0};
    int speed{0};

    int frameCounter{0};

    //save/load highscore
    std::ofstream data{};
    std::ifstream iData{"save.txt"};
    iData >> highscore;

    auto currentState{GameState::logo};

    while(!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(RAYWHITE);

        //Change track when one is over
        UpdateMusicStream(tracks[currentTrack]);
        timePlayed = GetMusicTimePlayed(tracks[currentTrack])/GetMusicTimeLength(tracks[currentTrack]);

        if (timePlayed >= 1.0f) {
            if(currentTrack < tracks.size() - 1) {
                StopMusicStream(tracks[currentTrack]);
                ++currentTrack;
            }
            else {
                StopMusicStream(tracks[currentTrack]);
                currentTrack = 0;
            }

            PlayMusicStream(tracks[currentTrack]);
        }

        switch(currentState) {
            case GameState::logo : {

                if(frameCounter > GetFPS() * 2) {
                    currentState = GameState::title;
                    frameCounter = 0;
                }

                DrawText("Loading", winHeight / 2, halfHeight, 124, DARKBLUE);
                DrawLine(3 * frameCounter, 0, 3 * frameCounter, winHeight, DARKBLUE);
                DrawLine(0, 2 * frameCounter, winWidth, 2 * frameCounter, DARKBLUE);

                frameCounter++;
            } break;

            case GameState::title : {

                if(IsKeyPressed(KEY_SPACE)) {
                    currentState = GameState::game;
                }
                DrawText("Dodger", winWidth * .25f, winHeight * .25, 124, DARKBLUE);
                DrawText("Press Space To Start", winWidth * .10f, winHeight / 2, 64, DARKBLUE);

            } break;

            case GameState::game : {

                distance = (180 - (speed * 2) ) * frameCounter / GetFPS();

                if(IsKeyReleased(KEY_W)) {
                    if(racerL.getUpdateInterval() > minSpeed) {
                        racerL.increaseUpdateInterval();
                        timeToSpawn = racerL.getUpdateInterval() * 2;
                    }
                    if(racerM.getUpdateInterval() > minSpeed) {
                        racerM.increaseUpdateInterval();
                        timeToSpawn = racerM.getUpdateInterval() * 2;
                    }
                    if(racerR.getUpdateInterval() > minSpeed) {
                        racerR.increaseUpdateInterval();
                        timeToSpawn = racerR.getUpdateInterval() * 2;
                    }
                }

                if(spawnTime > timeToSpawn) {
                    switch(GetRandomValue(1, 4)) {
                        case 1: racerL.spawn(); break;
                        case 2: racerM.spawn(); break;
                        case 3: racerR.spawn(); break;
                    }

                    spawnTime = 0;
                }

                if(racerL.getUpdateInterval() < racerM.getUpdateInterval() && racerR.getUpdateInterval()) {
                    speed = racerL.getUpdateInterval();
                } else if ( racerM.getUpdateInterval() < racerL.getUpdateInterval() && racerR.getUpdateInterval() ) {
                    speed = racerM.getUpdateInterval();
                } else
                {
                    speed = racerR.getUpdateInterval();
                }

                //background gradient
                DrawRectangleGradientV(0,0,winWidth, halfHeight, DARKBLUE, RAYWHITE);
                DrawRectangleGradientV(0, halfHeight, winWidth, winHeight, RAYWHITE, GRAY);

                checkMove(player, columns);
                player.draw();

                drawRoad(winHeight, winWidth, halfHeight, halfWidth , DARKBLUE);
                drawRoadMarkers(winHeight, winWidth, halfHeight, halfWidth , DARKBLUE);

                //draw buildings
                for(int i{0}; i < 10; ++i) {
                    int yPosition{i * 30};
                    DrawRectangleLines(10 * i * i, yPosition, 50 - i, halfHeight - yPosition, DARKBLUE );
                }
                for(int i{0}; i < 10; ++i) {
                    int yPosition{i * 30};
                    DrawRectangleLines(winWidth - (100 * i), yPosition, 50 + i, halfHeight - yPosition, DARKBLUE );
                }

                //draw stat trackers
                DrawText(TextFormat("Dodges: %i",score),winWidth * .70f,winHeight * .05f,36,RAYWHITE);
                DrawText(TextFormat("Speed: %imph",180 - (speed * 2)),winWidth * .70f,winHeight * .10f,36,RAYWHITE);
                DrawText(TextFormat("Distance: %i'",distance),winWidth * .70f,winHeight * .15f,36,RAYWHITE);

                controlRacer(racerL, currentState, player, score);
                controlRacer(racerM, currentState, player, score);
                controlRacer(racerR, currentState, player, score);

                ++spawnTime;
                ++frameCounter;
            } break;

            case GameState::end : {


                resetRacer(racerL);
                resetRacer(racerM);
                resetRacer(racerR);

                //save highscore
                if(score > highscore) {
                    highscore = score;
                    data.open("save.txt");
                    data << highscore;
                    data.close();
                }

                score = 0;
                timeToSpawn = baseRacerUpdateInterval * 2;
                speed = baseRacerUpdateInterval;
                frameCounter = 0;

                drawRoad(winHeight, winWidth, halfHeight, halfWidth , DARKBLUE);
                drawRoadMarkers(winHeight, winWidth, halfHeight, halfWidth , DARKBLUE);

                DrawText("Press R To Retry", winWidth * .35f, winHeight * .30f, 36, DARKBLUE);
                DrawText("Press ESC To Quit", winWidth * .35f, winHeight * .40f, 36, DARKBLUE);

                DrawText("Highscore:", winWidth * .40f, winHeight * .20f, 36, DARKBLUE);
                DrawText(TextFormat("%i",highscore),winWidth * .61f,winHeight * .20f,36,DARKBLUE);


                if(IsKeyPressed(KEY_R)) {
                    currentState = GameState::game;
                }

            } break;
        }

        EndDrawing();
    }

    UnloadMusicStream(tracks[currentTrack]);
    CloseAudioDevice();

    return 0;
}

void controlRacer(Racer& racer, GameState& currentState, Player& player, int& score) {
    if(racer.canSpawn()) {
        racer.drawRacer();
        racer.decrementMoveCooldown();

        if(racer.isColliding(player.getRec())) {
            currentState = GameState::end;
        }

        if(racer.getMoveCooldown() > racer.getUpdateInterval()) {
            racer.move();
            racer.resetMoveCooldown();

            if (racer.getPosition() > 4) {
                ++score;
                racer.resetPosition();
                racer.resetMoveCooldown();

                if(racer.getUpdateInterval() > 15) {
                    racer.increaseUpdateInterval();
                }

                racer.resetSpawn();
            }
        }
    }
}

void resetRacer(Racer& racer) {
    racer.resetPosition();
    racer.resetMoveCooldown();
    racer.resetSpawn();
    racer.resetUpdateInterval();
}

void checkMove(Player& player, int columns) {
    if(IsKeyPressed(KEY_A) && player.getRec().x > player.getRec().width) {
        player.moveLeft();
    }
    else if (IsKeyPressed(KEY_D) && player.getRec().x < player.getRec().width * (columns - 2)) {
        player.moveRight();
    }
}

void drawRoad(int winHeight, int winWidth, int halfHeight, int halfWidth , Color&& color) {
    //horizon line
    DrawLine(0, halfHeight, winWidth, halfHeight, color);

    //road
    DrawLine(halfWidth, halfHeight, 0, winHeight, color);        //left road edge
    DrawLine(halfWidth, halfHeight, winWidth, winHeight, color); //right road edge
}

void drawRoadMarkers(int winHeight, int winWidth, int halfHeight, int halfWidth , Color&& color) {
    DrawLine(halfWidth, halfHeight, winWidth * .30f, winHeight, color); // left lines
    DrawLine(halfWidth, halfHeight, winWidth * .70f, winHeight, color); // right lines
}

