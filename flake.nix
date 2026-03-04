{
  inputs.librelane.url = "github:librelane/librelane/dev";

  outputs = {
    self,
    librelane,
    ...
  }: let
    nix-eda = librelane.inputs.nix-eda;
    nixpkgs = nix-eda.inputs.nixpkgs;
    lib = nixpkgs.lib;
  in {
    legacyPackages = nix-eda.forAllSystems (
      system:
        import nix-eda.inputs.nixpkgs {
          inherit system;
          overlays = [
            nix-eda.overlays.default
            librelane.overlays.default
          ];
        }
    );

    devShells = nix-eda.forAllSystems (
      system: let
        pkgs = self.legacyPackages."${system}";
      in {
        default = pkgs.opensta.overrideAttrs (attrs': attrs: {
          nativeBuildInputs = with pkgs; attrs.nativeBuildInputs ++ [python3];
        });
      }
    );
  };
}
