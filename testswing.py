
from jep import *

from javax.swing import *
from java.awt import *
from java.awt.event import *

printStack(True)

# port of:
# http://java.sun.com/docs/books/tutorial/uiswing/learn/examples/SwingApplication.java

class SwingApplication:
    labelPrefix = "Number of button clicks: "
    numClicks   = 0
    label       = JLabel(labelPrefix + "0    ")

    def createComponents(self):
        button = JButton("I'm a Swing button!")
        button.setMnemonic(KeyEvent.VK_I)
        button.addActionListener(jproxy(self,
                                        ['java.awt.event.ActionListener']))
        self.label.setLabelFor(button)

        # An easy way to put space between a top-level container
        # and its contents is to put the contents in a JPanel
        # that has an "empty" border.

        pane = JPanel(GridLayout(0, 1))
        pane.add(button)
        pane.add(self.label)
        pane.setBorder(BorderFactory.createEmptyBorder(
            30, #top
            30, #left
            10, #bottom
            30)) #right

        return pane


    def actionPerformed(self, e):
        self.numClicks += 1
        self.label.setText(self.labelPrefix + str(self.numClicks))


# Create the GUI and show it.  For thread safety,
# this method should be invoked from the
# event-dispatching thread.
def createAndShowGUI():
    #Make sure we have nice window decorations.
    JFrame.setDefaultLookAndFeelDecorated(True)

    #Create and set up the window.
    frame = JFrame("SwingApplication")
    frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE)

    app = SwingApplication()
    contents = app.createComponents()
    frame.getContentPane().add(contents, BorderLayout.CENTER)

    #Display the window.
    frame.pack()
    frame.setVisible(True)


if(__name__ == '__main__'):
    #Schedule a job for the event-dispatching thread:
    #creating and showing this application's GUI.
    createAndShowGUI()
