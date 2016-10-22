#ifndef MAZE_H_
#define MAZE_H_

template <typename Display>
class Maze {
  Maze(Display &display) : display(display) {

  }

private:
  Display &display;
};
#endif /* !MAZE_H_ */
