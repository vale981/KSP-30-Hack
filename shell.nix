{ pkgs ? import <nixos> {} }:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [ arduino git arduino-cli picocom clang-tools ];
}
