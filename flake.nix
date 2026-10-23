{
  description = "Teleport Engine: software renderer with teleporting portals";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forAllSystems = f: nixpkgs.lib.genAttrs systems
        (system: f nixpkgs.legacyPackages.${system});
    in
    {
      packages = forAllSystems (pkgs: {
        default = pkgs.stdenv.mkDerivation {
          pname = "teleport-engine";
          version = "0.1.0";
          src = self;

          nativeBuildInputs = [ pkgs.pkg-config ];

          # The code targets the SFML 2 API; `sfml` is SFML 3
          buildInputs = [ pkgs.sfml_2 ];

          enableParallelBuilding = true;
          makeFlags = [
            "RESOURCE_DIR=${placeholder "out"}/share/teleport-engine"
          ];

          installPhase = ''
            runHook preInstall

            install -Dm755 build/teleport-engine \
              $out/bin/teleport-engine
            mkdir -p $out/share/teleport-engine
            cp -r Resources/. $out/share/teleport-engine/

            runHook postInstall
          '';

          meta.mainProgram = "teleport-engine";
        };
      });

      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          inputsFrom = [ self.packages.${pkgs.system}.default ];
        };
      });
    };
}
