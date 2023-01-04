//==============================================================================

#include "MainApplication.h"
#include "MainWindow.h"
#include "MainComponent.h"

MainWindow::MainWindow(String name)
: DocumentWindow(name, Colours::white, DocumentWindow::allButtons) {
  
  auto maincomp = std::make_unique<MainComponent>();
  setContentOwned(maincomp.release(), false);
  setResizable(true, true);
  setResizeLimits(600, 400, 1200, 800);
  centreWithSize(getWidth(), getHeight());
  setVisible(true);
}

//==============================================================================
// DocumentWindow overrides

void MainWindow::closeButtonPressed() {
  // when the main window is closed signal the app to exit
  JUCEApplication::getInstance()->systemRequestedQuit();
}

