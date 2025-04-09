#include "app.hpp"
#include <imgui.h>
#include <iostream>
#include <string>
#include "Chess/Board.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

void app::init() {
    // Initialiser dans l'ordre correct
    m_renderer3D.initialize(); 
    m_board.setRenderer3D(&m_renderer3D);
    m_board.initializeBoard(); 
    m_board.updateRenderer3D();
}

void app::update() {
    static float lastFrameTime = (float)glfwGetTime();
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    
    m_renderer3D.update(deltaTime);
    
    //FENETRE POUR LA CAM
    if (ImGui::Begin("Camera Controls")) {
        //Mode actuel de la cam
        const char* cameraMode = (m_renderer3D.getCamera().getCameraMode() == CameraMode::Trackball) ? 
                                "Mode Trackball" : "Mode Vue Pièce";
        ImGui::Text("Mode actuel: %s", cameraMode);
        
        // Instructions adaptées au mode
        if (m_renderer3D.getCamera().getCameraMode() == CameraMode::Trackball) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), //TODO : à changer avec une variable de couleur
                "Clic droit pour vous déplacer autour de l'échiquier");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), 
                "Vue depuis la pièce - utilisez le clic droit pour regarder autour");
        }
        
        // Sélection rapide des pièces
        if (ImGui::CollapsingHeader("Sélectionner une pièce pour la vue", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::Button("Roi blanc", ImVec2(100, 30))) {
                m_renderer3D.selectPieceForView(4, 0);
            }
            ImGui::SameLine();
            if (ImGui::Button("Dame blanche", ImVec2(100, 30))) {
                m_renderer3D.selectPieceForView(3, 0);
            }
            
            if (ImGui::Button("Roi noir", ImVec2(100, 30))) {
                m_renderer3D.selectPieceForView(4, 7);
            }
            ImGui::SameLine();
            if (ImGui::Button("Dame noire", ImVec2(100, 30))) {
                m_renderer3D.selectPieceForView(3, 7);
            }
        }
        
        // Bouton pour changer de mode caméra
        if (ImGui::Button("Changer de mode caméra", ImVec2(200, 30))) {
            m_renderer3D.toggleCameraMode();
        }
        
        // Aide des contrôles
        ImGui::Separator();
        ImGui::Text("Contrôles: Clic droit + déplacer pour tourner la caméra");
        ImGui::Text("Molette de souris pour zoomer (mode Trackball uniquement)");
        ImGui::Text("Touche C pour basculer rapidement entre les modes");
    }
    ImGui::End();
    
    // Fenêtre du rendu 3D
    ImGui::Begin("3D Chess Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        m_renderer3D.getCamera().setAspectRatio(viewportSize.x / std::max(viewportSize.y, 1.0f));
        
        glm::mat4 view = m_renderer3D.getCamera().getViewMatrix();
        glm::mat4 projection = m_renderer3D.getCamera().getProjectionMatrix();

        static GLuint fbo = 0, renderTexture = 0, depthBuffer = 0;
        if (fbo == 0) {
            // Création initiale des ressources de rendu
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            
            // Texture pour le résultat coloré
            glGenTextures(1, &renderTexture);
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)viewportSize.x, (int)viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
            
            // Buffer de profondeur
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)viewportSize.x, (int)viewportSize.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        // Gestion du redimensionnement
        static ImVec2 lastSize = viewportSize;
        if (lastSize.x != viewportSize.x || lastSize.y != viewportSize.y) {
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)viewportSize.x, (int)viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)viewportSize.x, (int)viewportSize.y);
            
            lastSize = viewportSize;
        }
        
        // Rendu de la scène dans le framebuffer
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
        
        // Affichage du résultat dans ImGui
        ImGui::Image((void*)(intptr_t)renderTexture, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
        
        // Gestion des contrôles de la caméra
        if (ImGui::IsItemHovered() && !ImGui::IsAnyItemActive()) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                // Rotation de la caméra avec la souris
                ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
                m_renderer3D.getCamera().rotateLeft(-mouseDelta.x * 0.01f);
                m_renderer3D.getCamera().rotateUp(mouseDelta.y * 0.01f);
            }
            
            // Zoom avec la molette (uniquement en mode trackball)
            if (m_renderer3D.getCamera().getCameraMode() == CameraMode::Trackball) {
                float wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0) {
                    m_renderer3D.getCamera().moveFront(wheel * 1.0f);
                }
            }
        }
    }
    
    ImGui::End();
    
    // Affichage de l'échiquier 2D
    m_board.drawBoard();
    
    // Gestion de la fin de partie
    static bool gameOverPopupClosed = false;
    
    if (m_board.isGameOver() && !gameOverPopupClosed) {
        ImGui::OpenPopup("Game Over");
    }

    if (ImGui::BeginPopupModal("Game Over", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        PieceColor winner = m_board.getWinner();
        const char* winnerText = (winner == PieceColor::White) ? "White" : "Black";
        ImGui::Text("%s wins! The king has been captured.", winnerText);

        // Options de fin de partie
        if (ImGui::Button("New Game", ImVec2(120, 0))) {
            // Réinitialisation complète du jeu
            m_board = Board();
            m_board.setRenderer3D(&m_renderer3D);
            m_board.initializeBoard();
            m_board.updateRenderer3D();
            
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
    
    // Raccourci pour changer de mode caméra
    if (ImGui::IsKeyPressed(ImGuiKey_C)) {
        m_renderer3D.toggleCameraMode();
    }
}