import displayio


class ScreenManager:
    """Manages a root group, a single active screen group, and optional overlays."""

    def __init__(self, display):
        self.display = display
        self.root = displayio.Group()
        # CP8 show() vs CP9 root_group
        try:
            display.show(self.root)
        except AttributeError:
            display.root_group = self.root
        self._screen = None
        self._overlays = []

    def show_screen(self, group: displayio.Group):
        if self._screen is not None and self._screen in self.root:
            self.root.remove(self._screen)
        self._screen = group
        self.root.append(group)

    def push_overlay(self, group: displayio.Group):
        self._overlays.append(group)
        self.root.append(group)

    def pop_overlay(self):
        if not self._overlays:
            return
        top = self._overlays.pop()
        if top in self.root:
            self.root.remove(top)

    @property
    def group(self):
        return self.root
