#include "AppModel.hpp"
#include "AppCLIController.hpp"
#include "AppCLIView.hpp"

int main(int argc, char** argv)
{
    AppModel model;
    AppCLIController controller(model);
    AppCLIView view(model, argc, argv);
    model.attach(&view);
    return view.show(controller);
}