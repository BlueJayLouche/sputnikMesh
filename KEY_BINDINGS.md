# Key Bindings Analysis

## Summary of Key Bindings

### Regular Keys (No Modifier)

| Key | Function | Notes |
|-----|----------|-------|
| `a` | Luma Key + | Also `Shift+A` for audio toggle ⚠️ |
| `z` | Luma Key - | |
| `s` | Z Frequency + | |
| `x` | Z Frequency - | |
| `d` | Z LFO Arg + | |
| `c` | Z LFO Arg - | |
| `f` | Z LFO Amp + | |
| `v` | Z LFO Amp - | |
| `g` | X Frequency + | |
| `b` | X Frequency - | |
| `h` | X LFO Arg + | |
| `n` | X LFO Arg - | |
| `j` | X LFO Amp + | |
| `m` | X LFO Amp - | Also `Shift+M` for MIDI template |
| `k` | Y Frequency + | |
| `,` | Y Frequency - | |
| `l` | Y LFO Arg + | |
| `.` | Y LFO Arg - | |
| `;` | Y LFO Amp + | |
| `/` | Y LFO Amp - | |
| `t` | Move Mesh X + | |
| `y` | Move Mesh X - | |
| `u` | Move Mesh Y + | |
| `i` | Move Mesh Y - | Also `Shift+I` for video info |
| `o` | Zoom + | |
| `p` | Zoom - | Also `Shift+P` for performance mode |
| `q` | X Displace + | |
| `w` | X Displace - | |
| `e` | Y Displace + | |
| `r` | Y Displace - | Also `Shift+R` for reset |
| `[` | Scale - | Also `Shift+[` for smoothing |
| `]` | Scale + | Also `Shift+]` for smoothing |
| `-` | Triangle mesh, no wireframe | Also `Shift+-` for amplitude |
| `=` | Triangle mesh, wireframe | Also `Shift+=` for amplitude |
| `1` | Toggle Luma Key | |
| `2` | Toggle Bright | |
| `3` | Toggle Invert | |
| `5` | Toggle B&W | ⚠️ Missing `4` key |
| `6` | Cycle Z LFO Shape | |
| `7` | Cycle X LFO Shape | |
| `8` | Cycle Y LFO Shape | |
| `9` | Vertical Lines mesh | |
| `0` | Horizontal Lines mesh | |
| `!` | Toggle Z Ring Mod | `Shift+1` |
| `@` | Toggle X Ring Mod | `Shift+2` |
| `#` | Toggle Y Ring Mod | `Shift+3` |
| `$` | Toggle Z Phase Mod | `Shift+4` |
| `%` | Toggle X Phase Mod | `Shift+5` |
| `^` | Toggle Y Phase Mod | `Shift+6` |
| `` ` `` | Toggle Debug | |
| `F5` | Save settings | Save current settings to settings.xml |
| `F9` | Load settings | Load settings from settings.xml |

### Shift+Key Combinations

| Key | Function |
|-----|----------|
| `Shift+A` | Toggle Audio ON/OFF |
| `Shift+M` | Generate MIDI mapping template |
| `Shift+P` | Toggle Performance Mode |
| `Shift+R` | Reset all parameters |
| `Shift+F` | Toggle fullscreen |
| `Shift+V` | Cycle video devices |
| `Shift+C` | Cycle video formats |
| `Shift+Z` | Cycle video resolutions |
| `Shift+I` | Print video info |
| `Shift+D` | Cycle audio devices |
| `Shift+N` | Toggle FFT normalization |
| `Shift+[` | Decrease audio smoothing |
| `Shift+]` | Increase audio smoothing |
| `Shift+-` | Decrease audio amplitude |
| `Shift+=` | Increase audio amplitude |
| `Shift+Ctrl+-` | Decrease frame rate |
| `Shift+Ctrl+=` | Increase frame rate |

## Identified Issues

### ⚠️ Conflicts & Problems

### ✅ Fixed Issues

1. **~~No `4` key binding~~** - ✅ Now toggles Wireframe
2. **~~README out of date~~** - Updated
3. **~~No help key~~** - ✅ Now `?` (Shift+/) toggles help overlay
4. **~~No exit key~~** - ✅ Now `ESC` exits gracefully

### 🔧 Recommended Fixes

1. Add a `4` key toggle (suggestion: Toggle Wireframe)
2. Update README.md with correct key bindings
3. Add `h` or `?` key for on-screen help
4. Add `Esc` key to exit gracefully

## Key Groups (Logical Organization)

### QWERTY Row - Displacement
- `q/w` - X Displace
- `e/r` - Y Displace  
- `t/y` - Move Mesh X
- `u/i` - Move Mesh Y
- `o/p` - Zoom

### ASDF Row - Z Controls
- `a/z` - Luma Key
- `s/x` - Z Frequency
- `d/c` - Z LFO Arg
- `f/v` - Z LFO Amp

### GHJKL Row - X Controls
- `g/b` - X Frequency
- `h/n` - X LFO Arg
- `j/m` - X LFO Amp

### Bottom Row - Y Controls
- `k/,` - Y Frequency
- `l/.` - Y LFO Arg
- `;/` - Y LFO Amp

### Number Row - Toggles & Shapes
- `1-5` - Toggles (Luma, Bright, Invert, ?, B&W)
- `6-8` - LFO Shapes (Z, X, Y)
- `9-0` - Mesh Types
- `!@#$%^` - Modulation toggles (with Shift)

### Bracket/Minus Row - Scale & Audio
- `[/]` - Mesh Resolution / Audio Smoothing (Shift)
- `-/=` - Mesh Type / Audio Amplitude (Shift)
