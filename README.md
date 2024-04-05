# Tool Chain Install (MACOS M2 Sillicon)

Download `darwin-x86_64-arm-none-eabi.pkg` from https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads. Find the installed location with `pkgutil --pkg-info arm-gnu-toolchain-13.2.Rel1-darwin-x86_64-arm-none-eabi` which should output something like this:

```
package-id: arm-gnu-toolchain-13.2.Rel1-darwin-x86_64-arm-none-eabi
version: 13.2.Rel1
volume: /
location: Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/
install-time: 1712341508
```

Modify CmakeLists.txt
