using Caliburn.Micro;

namespace TargetControl
{
    public sealed class ManualControlViewModel : Screen, IMainScreenTabItem
    {
        public ManualControlViewModel()
        {
            DisplayName = "Manual";
        }
    }
}