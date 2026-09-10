{ stdenv, clang }:

stdenv.mkDerivation {
  pname = "cbld";
  version = "0.1.0";

  src = ./.;

  nativeBuildInputs = [
    clang
  ];

  buildPhase = ''
    clang -std=c99 -Wall -Wextra -O2 src/cbld.c -o cbld
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp cbld $out/bin/
  '';
}
