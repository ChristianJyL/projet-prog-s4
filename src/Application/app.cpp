#include "app.hpp"
#include <imgui.h>
#include <iostream>
#include <string>
#include "Chess/Board.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include "../Chess/GameMode/ClassicChess.hpp"
#include "../Chess/GameMode/DrunkChess.hpp"

void app::init() {
    m_renderer3D.initialize(); 
    m_board.initializeBoard(&m_renderer3D);
}

void app::drawGameModeWindow() {
    if (ImGui::CollapsingHeader("Mode de Jeu")) {
        if (ImGui::Button("Mode Classique", ImVec2(150, 30))) {
            m_board = Board();
            m_board.setGameMode(std::make_unique<ClassicChessMode>());
            m_board.initializeBoard(&m_renderer3D);
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Sous quelques grammes", ImVec2(200, 30))) {
            m_board = Board();
            m_board.setGameMode(std::make_unique<DrunkChessMode>());
            m_board.initializeBoard(&m_renderer3D);
        }
        
        ImGui::Separator();
        ImGui::Text("Mode actuel: %s", m_board.getCurrentModeName().c_str());

        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Attention: L'abus d'alcool est dangereux pour la santé!");
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "À consommer avec modération!");
    }
    
    if (m_board.getGameMode()) {
        m_board.getGameMode()->drawModeSpecificUI();
    }
}

void app::drawCameraControlWindow() {
    if (ImGui::CollapsingHeader("Caméra")) {
        const char* cameraMode = (m_renderer3D.getCameraMode() == CameraMode::Trackball) ? "Mode Trackball" : "Mode Vue Pièce";
        ImGui::Text("Mode actuel: %s", cameraMode); 
        
        if (m_renderer3D.getCameraMode() == CameraMode::Trackball) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),  
                "Clic droit pour vous déplacer autour de l'échiquier");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 
                "Vue depuis la pièce - utilisez le clic droit pour regarder autour");
        }
        
        if (ImGui::Button("Changer de mode caméra", ImVec2(200, 30))) {
            m_renderer3D.toggleCameraMode();
        }
    }
}

void app::draw3DViewportWindow() {
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        m_renderer3D.setCameraAspectRatio(viewportSize.x / std::max(viewportSize.y, 1.0f));
        
        glm::mat4 view = m_renderer3D.getViewMatrix();
        glm::mat4 projection = m_renderer3D.getProjectionMatrix();

        static GLuint fbo = 0, renderTexture = 0, depthBuffer = 0;
        if (fbo == 0) {
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            
            glGenTextures(1, &renderTexture);
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)viewportSize.x, (int)viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
            
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)viewportSize.x, (int)viewportSize.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        static ImVec2 lastSize = viewportSize;
        if (lastSize.x != viewportSize.x || lastSize.y != viewportSize.y) {
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)viewportSize.x, (int)viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)viewportSize.x, (int)viewportSize.y);
            
            lastSize = viewportSize;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, (int)viewportSize.x, (int)viewportSize.y);
        
        if (m_renderer3D.isInitialized()) {
            try {
                m_renderer3D.render();
            } catch (const std::exception& e) {
                std::cerr << "Erreur lors du rendu: " << e.what() << std::endl;
            }
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        ImGui::Image((void*)(intptr_t)renderTexture, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
        
        if (ImGui::IsItemHovered() && !ImGui::IsAnyItemActive()) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
                m_renderer3D.rotateCameraLeft(-mouseDelta.x * 0.01f);
                m_renderer3D.rotateCameraUp(mouseDelta.y * 0.01f);
            }
            
            if (m_renderer3D.getCameraMode() == CameraMode::Trackball) {
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0) {
                    m_renderer3D.moveCameraFront(wheel * 1.0f);
                }
            }
        }
    }
}

void app::drawGameOverPopup(bool& gameOverPopupClosed) {
    if (m_board.isGameOver() && !gameOverPopupClosed) {
        ImGui::OpenPopup("Game Over");
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Game Over", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        PieceColor winner = m_board.getWinner();
        const char* winnerText = (winner == PieceColor::White) ? "White" : "Black";
        ImGui::Text("%s wins! The king has been captured.", winnerText);

        if (ImGui::Button("New Game", ImVec2(120, 0))) {
            m_board = Board();
            m_board.initializeBoard(&m_renderer3D);
            
            gameOverPopupClosed = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Close", ImVec2(120, 0))) {
            gameOverPopupClosed = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void app::update() {
    static float lastFrameTime = (float)glfwGetTime();
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    
    m_renderer3D.update(deltaTime);
    
    static bool gameOverPopupClosed = false;

    if (ImGui::Begin("Vrai vue 3D")) {
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        
        //on désactive les bords
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
        
        float panelWidth = windowSize.x * 0.7f;
        
        ImGui::BeginChild("Vue3D", ImVec2(panelWidth, 0), false);
        draw3DViewportWindow();
        ImGui::EndChild();
        
        ImGui::SameLine();
        
        ImGui::BeginChild("Controles", ImVec2(0, 0), false);
        
        if (m_board.isGameOver()) {
            PieceColor winner = m_board.getWinner();
            const char* winnerText = (winner == PieceColor::White) ? "Blanc" : "Noir";
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "Partie terminée!");
            ImGui::Text("Vainqueur: %s", winnerText);
            ImGui::Separator();
        }
        
        if (ImGui::CollapsingHeader("Échiquier 2D", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Mode de jeu: %s", m_board.getCurrentModeName().c_str());
            
            float panelWidth = ImGui::GetContentRegionAvail().x;
            ImGui::BeginChild("Chess2D", ImVec2(panelWidth, panelWidth), true, ImGuiWindowFlags_NoScrollbar);
            
            m_board.drawBoard();
            
            ImGui::EndChild();
        }
        
        drawGameModeWindow();
        drawCameraControlWindow();
        
        ImGui::EndChild();
        
        // Restaurer le style par défaut
        ImGui::PopStyleVar();
    }
    ImGui::End();

    drawGameOverPopup(gameOverPopupClosed);

    if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        m_renderer3D.toggleCameraMode();
    }
}
