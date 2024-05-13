{
  description = "Simple Notebook";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs, ... }@inputs:
    let
      pkgs = import nixpkgs {
        system = "x86_64-linux";
        config.allowUnfree = true;
      };
      # Dev time (developing tools)
      devInputs = with pkgs; [
        valgrind
      ];
      # Build time (build tools; header libs)
      nativeBuildInputs = with pkgs; [
        cmake
        clang-tools
      ];
      # Run time (libs to link with)
      buildInputs = with pkgs; [
        SDL2
        SDL2_ttf
        SDL2_image
        cairo
      ];

    in {
    # Utilized by `nix develop`
    devShell.x86_64-linux = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
      inherit buildInputs;
      nativeBuildInputs = nativeBuildInputs ++ devInputs;
    };

    # Utilized by `nix build`
    defaultPackage.x86_64-linux = pkgs.clangStdenv.mkDerivation rec {
      pname = "notebook";
      version = "0.1.0";
      src = ./.;

      inherit nativeBuildInputs;
      inherit buildInputs;

      buildPhase = "make -j $NIX_BUILD_CORES";

      installPhase = ''
        runHook preInstall
        ls -aR
        install -m755 -D notebook $out/bin/notebook
        runHook postInstall
      '';
    };

    # Utilized by `nix run`
    apps.x86_64-linux = {
      type = "app";
      program = self.packages.x86_64-linux.notebook;
    };
  };
}
