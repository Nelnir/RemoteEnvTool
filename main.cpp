#include "AppModel.hpp"
#include "AppCLIController.hpp"
#include "AppCLIView.hpp"
int main(int argc, char** argv)
{
    AppModel model;
    AppCLIController controller(model);
    AppCLIView view(argc, argv);
    view.show(controller);
    return 0;
}