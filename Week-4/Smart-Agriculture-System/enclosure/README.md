# 3D-Printed Enclosure Plan

## Mechanical Concept

A two-piece, splash-resistant electronics enclosure with a removable lid:

```text
          FRONT VIEW
   +-----------------------+
   |   SMART AGRICULTURE   |
   |  +-----------------+  |
   |  | OLED 128 x 64   |  |
   |  +-----------------+  |
   +-----------------------+

          SIDE / CABLE VIEW
   +-----------------------+
   | PCB + ESP32           |---- cable gland -> sensors
   |                       |
   |  dry electronics      |---- cable gland -> pump driver
   +-----------------------+
```

## Suggested Starting Dimensions

- Outer body: approximately 140 × 100 × 55 mm.
- Wall thickness: 2.5–3 mm.
- Lid: 3 mm minimum.
- Four M3 mounting bosses for the PCB.
- Four M3 lid screws with heat-set inserts or captive nuts.
- OLED window sized after measuring the actual module bezel; do not rely only on the nominal 128 × 64 display resolution.

These are starting dimensions only. Final dimensions should be driven by the selected PCB, connector clearances, battery, and pump wiring.

## Features

- Front OLED cutout with an internal bezel/lip.
- Side cable-gland holes for sensor leads and pump-control wiring.
- Ventilation slots positioned away from direct water paths.
- Internal standoffs to keep the PCB off the enclosure floor.
- Drip edge around the lid seam.
- Optional gasket channel around the lid.
- Small drainage/weep path at the lowest point so accidental condensation does not pool around electronics.
- Separate external pump wiring route so wet tubing never passes through the electronics compartment.

## Printing Guidance

- PETG or ASA is preferred for outdoor prototypes; PLA is better suited to indoor testing.
- Use 0.2 mm layer height as a reasonable starting point.
- Use sufficient wall/perimeter count for screw bosses and cable-gland loads.
- Print the lid separately from the body.
- Test-print only the OLED cutout and mounting-hole pattern before printing the full enclosure.

## Environmental Notes

The printed box is **not automatically waterproof**. Outdoor deployment requires appropriate IP-rated cable glands, a gasket/seal strategy, UV/temperature-resistant material, drip loops, and validation in the actual installation environment.

Do not place the enclosure where irrigation spray can directly hit the OLED opening, cable entries, or lid seam.

## CAD Workflow

1. Measure the final PCB and all connector protrusions.
2. Import the PCB outline into a CAD tool such as FreeCAD, Fusion, or Onshape.
3. Place mounting bosses at the actual PCB hole coordinates.
4. Add the OLED window and check viewing angle.
5. Add cable glands with the exact gland thread diameter.
6. Add lid overlap, screw bosses, and gasket channel if required.
7. Export STL and perform a dimensional test print.
8. Assemble without electronics first; then perform a dry electrical test.

No fabricated STL is claimed by this plan until the final PCB and component dimensions are frozen.