let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-25.11";
  pkgs = import nixpkgs { config = {}; overlays = []; };
in
pkgs.mkShellNoCC {
  packages = with pkgs; [
    gforth
    python3
    pypy3
    sbcl
    chicken
    chez
    newlisp
    racket
    tcl
    lua
    luajit
    swi-prolog
    openjdk
    rustc
  ];
}
